/*
 * Copyright (c) 2016 Francesco Balducci
 *
 * This file is part of arduino_c.
 *
 *    arduino_c is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    arduino_c is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with arduino_c.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>

static
void sd_select(void)
{
    _delay_us(100);
    PORTD &= ~_BV(PORTD4); /* SD_CS low */
    _delay_us(100);
}

static
void sd_deselect(void)
{
    _delay_us(100);
    PORTD |= _BV(PORTD4); /* SD_CS high */
    _delay_us(100);
}

static
uint8_t spi_xfer(uint8_t tx)
{
    uint8_t rx;

    /* assuming last transfer was done so SPIF is 0 */
    SPDR = tx;
    loop_until_bit_is_set(SPSR, SPIF);
    rx = SPDR;

    printf("tx:%02x rx:%02x\n", tx, rx); 

    return rx;
}

static
uint8_t crc7_get(uint8_t cmd, uint32_t arg)
{
    uint8_t crc7;

    (void)arg;
    cmd &= 0x3F;
    if (cmd == 0)
    {
        /* arg should be 0 */
        crc7 = 0x94;
    }
    else if (cmd == 8)
    {
        /* arg should be 0x1AA */
        crc7 = 0x86;
    }
    else
    {
        /* don't care */
        crc7 = 0xFF;
    }
    crc7 |= 0x01; /* end bit */
    return crc7;
}

static
void send_cmd(uint8_t cmd, uint32_t arg)
{
    uint8_t crc7;

    crc7 = crc7_get(cmd, arg);

    cmd |= 0x40;
    (void)spi_xfer(cmd);
    (void)spi_xfer((arg>>24) & 0xFF);
    (void)spi_xfer((arg>>16) & 0xFF);
    (void)spi_xfer((arg>> 8) & 0xFF);
    (void)spi_xfer((arg>> 0) & 0xFF);
    (void)spi_xfer(crc7);
}

void sd_send_command(uint8_t cmd, uint32_t arg, void *resp, size_t len)
{
    uint8_t *resp_bytes;
    size_t i_byte;

    sd_select();
    send_cmd(cmd, arg);
    resp_bytes = resp;

    for (i_byte = 0; i_byte < len; i_byte++)
    {
        uint8_t r;
        do
        {
            r = spi_xfer(0xFF);
        } while (((r & 0x80) == 0x80) && (i_byte == 0));

        resp_bytes[i_byte] = r;
    }

    sd_deselect();
    (void)spi_xfer(0xFF);
}

uint8_t sd_send_command_r1(uint8_t cmd, uint32_t arg)
{
    uint8_t r1;

    sd_send_command(cmd, arg, &r1, 1);

    return r1;
}

extern
uint8_t sd_read_single_block(uint32_t address, void *dst)
{
    uint8_t *dst_bytes;
    uint8_t r1;
    uint8_t data_ctrl;

    sd_select();
    send_cmd(17, address);
    dst_bytes = dst;

    do
    {
        r1 = spi_xfer(0xFF);
    } while (r1 & 0x80);
    
    do
    {
        data_ctrl = spi_xfer(0xFF);
    } while (data_ctrl == 0xFF);
    if (data_ctrl == 0xFE)
    {
        int i_byte;
        uint8_t crc16_hi;
        uint8_t crc16_lo;

        data_ctrl = 0x00;
        for (i_byte = 0; i_byte < 512; i_byte++)
        {
            dst_bytes[i_byte] = spi_xfer(0xFF);
        }
        crc16_hi = spi_xfer(0xFF);
        crc16_lo = spi_xfer(0xFF);
        (void)crc16_hi;
        (void)crc16_lo;
    }

    sd_deselect();
    (void)spi_xfer(0xFF);

    return data_ctrl;
}

static
void sd_init(void)
{
    int i_dummy;

    DDRB |=  _BV(DDB5); /* SCK */
    DDRB &= ~_BV(DDB4); /* MISO */
    DDRB |=  _BV(DDB3); /* MOSI */
    DDRB |=  _BV(DDB2); /* SS (Wiznet) */
    DDRD |=  _BV(DDD4); /* SD_CS */

    PORTB &= ~_BV(PORTB4); /* MISO pull-up disable */
    PORTB |=  _BV(PORTB2); /* WZ_SS high */
    sd_deselect();

    SPCR =  /* mode 0, MSB first */
          _BV(MSTR)
        | _BV(SPE)
        | _BV(SPR0) | _BV(SPR0); /* 16MHz / 128 -> 125kHz */
    
    _delay_ms(1);
    for (i_dummy = 0; i_dummy < 80; i_dummy++)
    {
        (void)spi_xfer(0xFF);
    }
}

static
void print_resp(uint8_t cmd, void *resp, size_t len)
{
    size_t i;
    uint8_t *resp_bytes;

    printf("CMD%d: ", cmd);
    resp_bytes = resp;
    for (i = 0; i < len; i++)
    {
        printf("%02X", (unsigned)resp_bytes[i]);
    }
    printf("\n");
}

int main(void)
{
    uint8_t r1;
    uint8_t r7[5];
    uint8_t r3[5];
    uint8_t r2[2];
    uint32_t arg_hcs;
    uint8_t block[512];

    
    printf("SD card SPI initialization...");
    getchar();
    printf("\n");

    sd_init();

    r1 = sd_send_command_r1(0, 0);
    print_resp(0, &r1, 1);
    if (r1 != 0x01)
    {
        fprintf(stderr, "state not idle\n");
        return 1;
    }

    sd_send_command(8, 0x1AA, r7, sizeof(r7));
    print_resp(8, r7, sizeof(r7));

    if (r7[0] != 0x01)
    {
        fprintf(stderr, "state not idle\n");
        return 1;
    }
    else if ((r7[3]&0x0F) != 0x01)
    {
        fprintf(stderr, "non supported voltage range\n");
        return 1;
    }
    else if (r7[4] != 0xAA)
    {
        fprintf(stderr, "check pattern error\n");
        return 1;
    }

    sd_send_command(58, 0, r3, sizeof(r3));
    print_resp(58, r3, sizeof(r3));

    arg_hcs = 0x40000000;
    do 
    {
        r1 = sd_send_command_r1(55, 0);
        r1 = sd_send_command_r1(41, arg_hcs);
    } while(r1 & 0x01);
    print_resp(41, &r1, 1);

    sd_send_command(58, 0, r3, sizeof(r3));
    print_resp(58, r3, sizeof(r3));
    if (r3[1] & 0x40)
    {
        printf("High capacity\n");
    }
    else
    {
        printf("Standard capacity\n");
    }

    sd_send_command(13, 0, r2, sizeof(r2));
    print_resp(13, r2, sizeof(r2));

    r1 = sd_read_single_block(0, block);
    if (r1 == 0)
    {
        print_resp(17, block, sizeof(block));
    }
    else
    {
        print_resp(17, &r1, 1);
    }

    sd_send_command(13, 0, r2, sizeof(r2));
    print_resp(13, r2, sizeof(r2));

    sd_send_command(13, 0, r2, sizeof(r2));
    print_resp(13, r2, sizeof(r2));

    return 0; 
}

