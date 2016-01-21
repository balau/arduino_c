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
#include <stdio.h>
#include <avr/io.h>

#define BAUD 57600
#include <util/setbaud.h>

static
FILE *stdio_usart0_file;

static
int stdio_usart0_put(char c, FILE *f)
{
    (void)f; /* ignored */

    if (c == '\n')
    {
        int r;

        r = stdio_usart0_put('\r', f);
        if (r != 0)
        {
            return r;
        }
    }
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;

    return 0;
}

static
int stdio_usart0_get(FILE *f)
{
    int c;

    (void)f; /* ignored */

    loop_until_bit_is_set(UCSR0A, RXC0);
    c = UDR0;

    return c;
}

__attribute__((constructor))
void stdio_usart0_init(void)
{
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~_BV(U2X0);
#endif
    UCSR0B = _BV(TXEN0) | _BV(RXEN0);

    stdio_usart0_file = fdevopen(
            stdio_usart0_put,
            stdio_usart0_get);
}

