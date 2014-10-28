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
#include <avr/io.h>
#include <stdint.h>
#include "ledmatrix.h"

static
const uint8_t COL_MASK =
          _BV(DDB0)
        | _BV(DDB1)
        | _BV(DDB2)
        | _BV(DDB3)
        | _BV(DDB4);

static
const uint8_t ROW_MASK_D = 
          _BV(DDD2)
        | _BV(DDD3)
        | _BV(DDD4)
        | _BV(DDD5)
        | _BV(DDD6)
        | _BV(DDD7);

static
const uint8_t ROW_MASK_B = 
          _BV(DDB5);

static
void cols_off(void)
{
    DDRB &= ~COL_MASK;
}

static
void col_on(int i_col)
{
    DDRB |= _BV(i_col);
}

static
void cols_setup(void)
{
    cols_off();
    PORTB &= ~COL_MASK; /* disable pull-up */
}

static
void rows_setup(void)
{
    DDRD |= ROW_MASK_D;
    PORTD &= ~ROW_MASK_D; /* everything off */
    DDRB |= ROW_MASK_B;
    PORTB &= ~ROW_MASK_B; /* everything off */
}

static
uint8_t dots_portd(uint8_t dots)
{
    uint8_t dots_d;
    dots_d = dots & 0x7E; /* bit 0 is on PB5 */
    dots_d |= (dots & 0x02)?_BV(DDD7):0x00; /* bit 1 is on PD7 */
    dots_d &= ~0x02;
    return dots_d;
}

static
uint8_t dots_portb(uint8_t dots)
{
    uint8_t dots_b;
    dots_b = (dots & 0x01)?_BV(DDB5):0x00;
    return dots_b;
}

static
void draw_dots_portd(uint8_t dots)
{
    uint8_t dots_d = dots_portd(dots);
    uint8_t dots_on = ROW_MASK_D & dots_d;
    uint8_t dots_off = ROW_MASK_D & ~dots_d;
    PORTD |= dots_on;
    PORTD &= ~dots_off;
}

static
void draw_dots_portb(uint8_t dots)
{
    uint8_t dots_b = dots_portb(dots);
    uint8_t dots_on = ROW_MASK_B & dots_b;
    uint8_t dots_off = ROW_MASK_B & ~dots_b;
    PORTB |= dots_on;
    PORTB &= ~dots_off;
}

static
void draw_dots(uint8_t dots)
{
    draw_dots_portd(dots);
    draw_dots_portb(dots);
}

static
void ledmatrix_draw_col(int i_col, uint8_t dots)
{
    cols_off();
    draw_dots(dots);
    col_on(i_col);
}

void ledmatrix_setup(void)
{
	cols_setup();
    rows_setup();
}

void ledmatrix_draw_next_subframe(const struct ledmatrix_frame *f)
{
    static int i_next_col = 0;
    static int i_next_row = 0;

    uint8_t dots;
    uint8_t dots_mask = (1<<N_DOTS_ON_MAX)-1;

    dots = f->cols[i_next_col];
    dots_mask <<= i_next_row;
    dots &= dots_mask;
    ledmatrix_draw_col(i_next_col, dots);
    i_next_row += N_DOTS_ON_MAX;
    if (i_next_row >= N_ROWS)
    {
        i_next_row = 0;
        i_next_col++;
        if (i_next_col == N_COLS)
        {
            i_next_col = 0;
        }
    }
}

