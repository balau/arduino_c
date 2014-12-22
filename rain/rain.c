/*
 * Copyright (c) 2014 Francesco Balducci
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
#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#define BVV(bit, val) ((val)?_BV(bit):0)

static void rain_init(void)
{
    DDRD &= ~_BV(DDD7); /* D pin connected to PORTD7 */
    DDRC &= ~_BV(DDC0); /* A pin connected to PORTC0(ADC0) */
    ADMUX =
          BVV(MUX0, 0) | BVV(MUX1, 0) | BVV(MUX2, 0) | BVV(MUX3, 0) /* Read ADC0 */
        | BVV(ADLAR, 1) /* Left justify */
        | BVV(REFS0, 1) | BVV(REFS1, 0); /* AVCC as reference voltage */
    ADCSRA =
          BVV(ADPS0, 1) | BVV(ADPS1, 1) | BVV(ADPS2, 1) /* clk/128 */
        | BVV(ADIE, 0) | BVV(ADIF, 0) /* No interrupt */
        | BVV(ADATE, 0) /* No auto-update */
        | BVV(ADSC, 0) /* Don't start conversion */
        | BVV(ADEN, 1); /* Enable */
}

static bool rain_get_d(void)
{
    return bit_is_set(PIND, PIND7);
}

static bool rain_is_raining(void)
{
    return !rain_get_d(); /* d = 0 means rain */
}

static uint8_t rain_get_a(void)
{
    ADCSRA |= _BV(ADSC); /* Start conversion */
    loop_until_bit_is_clear(ADCSRA, ADSC); /* Wait for end of conversion */
    return ADCH;
}

static uint8_t rain_get_humidity(void)
{
    return 255 - rain_get_a(); /* lower means more humidity */
}

static void pwm_set_ocr(uint8_t oc)
{
    OCR0A = oc;
}

static void pwm_init(uint8_t oc)
{
	DDRD |= _BV(DDD6); /* set pin 6 of PORTD for output*/
    TCCR0A = 
          BVV(WGM00, 1) | BVV(WGM01, 1) /* Fast PWM update on OCRA */
        | BVV(COM0A1, 1) | BVV(COM0A0, 0) /* non-inverting OC0A */
        | BVV(COM0B1, 0) | BVV(COM0B0, 0); /* OC0B not connected */
    pwm_set_ocr(oc);
    TCCR0B =
          BVV(CS00, 1) | BVV(CS01, 0) | BVV(CS02, 1) /* F_CPU/1024 */
        | BVV(WGM02, 0) /* Fast PWM update on OCRA */
        | BVV(FOC0A, 0) | BVV(FOC0B, 0); /* ignored */
}

#define BAUD 57600
#include <util/setbaud.h>

static void usart_init(void)
{
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    UCSR0B = BVV(TXEN0, 1) | BVV(RXEN0, 0); /* Only TX */
}

static void usart_tx(char c) {
    while(!(UCSR0A & _BV(UDRE0)));
    UDR0 = c;
}

static void usart_puts(const char *s)
{
    while(*s != '\0')
    {
        usart_tx(*s++);
    }
}

static void gauge(char *dst, uint8_t size, uint8_t val, uint8_t max_val)
{
    uint8_t i;
    uint8_t levels;
    uint8_t gauge_level;

    levels = size - 2 - 1; /* start/end markers and null char */
    gauge_level = (val * (uint16_t)levels) / max_val;
    *dst++ = '[';
    for(i = 0; i < levels; i++)
    {
        char c;
        
        c = (i < gauge_level)?'=':' ';
        *dst++ = c;
    }
    *dst++ = ']';
    *dst = '\0';
}

static void byte2hex(char *dst, uint8_t src)
{
    const char hexdigits[16] = "0123456789ABCDEF";
    
    *dst++ = hexdigits[(src >> 4) & 0xF];
    *dst++ = hexdigits[src & 0xF];
    *dst = '\0';
}

static void update_gauge(bool state, uint8_t lvl)
{
    const uint8_t gauge_strlen = 50;
    char line[1+1+gauge_strlen+2+1];
    char *pline = &line[0];

    *pline++ = '\r';
    *pline++ = state?'1':'0';
    gauge(pline, gauge_strlen+1, lvl, 255);
    pline += gauge_strlen;
    byte2hex(pline, lvl);
    pline += 2;
    *pline++ = '\0';
    usart_puts(line);
}

int main (void)
{
    rain_init();
    usart_init();
    pwm_init(0);
    
    while (true)
    {
        bool raining;
        uint8_t humidity;

        raining = rain_is_raining();
        humidity = rain_get_humidity();

        pwm_set_ocr(humidity);
        update_gauge(raining, humidity);
        _delay_ms(100);
    }
}

