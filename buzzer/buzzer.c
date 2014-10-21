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
#include <util/delay.h>

enum t0_prescaler
{
    T0_PRESCALER_1 = _BV(CS00),
    T0_PRESCALER_8 = _BV(CS01),
    T0_PRESCALER_64 = _BV(CS00) | _BV(CS01),
    T0_PRESCALER_256 = _BV(CS02),
    T0_PRESCALER_1024 = _BV(CS02) | _BV(CS00),
};

static void t0_set_prescaler(enum t0_prescaler ps)
{
    TCCR0B = ps;
}

static unsigned short t0_get_prescaler_rate(enum t0_prescaler ps)
{
    unsigned short rate;
    switch(ps)
    {
        case T0_PRESCALER_1:
            rate = 1;
            break;
        case T0_PRESCALER_8:
            rate = 8;
            break;
        case T0_PRESCALER_64:
            rate = 64;
            break;
        case T0_PRESCALER_256:
            rate = 256;
            break;
        case T0_PRESCALER_1024:
            rate = 1024;
            break;
        default:
            rate = 0;
            break;
    }
    return rate;
}

static unsigned long div_round(unsigned long d, unsigned long q)
{
    return (d + (q/2)) / q;
}

static void t0_set_ctc_a(unsigned long hz, unsigned long timer_freq)
{
    OCR0A = div_round(timer_freq, hz*2) - 1;
    TCCR0A =
          _BV(COM0A0) // toggle
        | _BV(WGM01); // CTC
}

int main(void)
{
    unsigned long timer_freq;
    enum t0_prescaler ps = T0_PRESCALER_256;

    DDRD |= _BV(DDD6);
    t0_set_prescaler(ps);
    timer_freq = div_round(F_CPU, t0_get_prescaler_rate(ps));

    while(1)
    {
        t0_set_ctc_a(440, timer_freq);
        _delay_ms(200);
        t0_set_ctc_a(880, timer_freq);
        _delay_ms(200);
    }
    return 0;
}

