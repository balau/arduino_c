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
#include <avr/interrupt.h>

#define T1_MAX 0xFFFFUL
#define T1_TICK_US 64
#define T1_PRESCALER (_BV(CS10)|_BV(CS12)) /* 1024 */

static
void light(int on_off);

static
void light_toggle(void);

static
void light_init(void);


static
void timer_stop(void)
{
    TCCR1B &= ~(_BV(CS10)|_BV(CS11)|_BV(CS12));
    TIMSK1 &= ~_BV(TOIE1);
    TIFR1 |= _BV(TOV1);
}

static
void timer_init(void)
{
    /* normal mode */
    TCCR1A &= ~(_BV(WGM10)|_BV(WGM11));
    TCCR1B &= ~(_BV(WGM13)|_BV(WGM12));
    timer_stop();
}

static
void timer_start(unsigned long us)
{
    unsigned long ticks;
    unsigned short tcnt;

    ticks = us / T1_TICK_US;

    if (ticks > T1_MAX)
    {
        tcnt = 0;
    }
    else
    {
        tcnt = -ticks-1;
    }
    TCNT1 = tcnt;
    /* enable interrupt */
    TIMSK1 |= _BV(TOIE1);
    /* start */
    TCCR1B &= ~(_BV(CS10)|_BV(CS11)|_BV(CS12));
    TCCR1B |= T1_PRESCALER;
}

static
void timer_start_ms(unsigned short ms)
{
    timer_start(ms * 1000UL);
}

static int switch_on = 0;

ISR(TIMER1_OVF_vect)
{
    timer_stop();
    if (bit_is_clear(PINB, PINB4))
    {
        /* button pressed */
        light(1);
        switch_on = 1;
    }
    else
    {
        if (switch_on)
        {
            timer_start_ms(20000);
            switch_on = 0;
        }
        else
        {
            light(0);
        }
    }
}

static
void light(int on_off)
{
    if (on_off)
    {
        PORTB |= _BV(PORTB5);
    }
    else
    {
		PORTB &= ~_BV(PORTB5);
    }
}

static
void light_toggle(void)
{
    light(bit_is_clear(PORTB, PORTB5));
}

static
void light_init(void)
{
	/* set pin 5 of PORTB for output*/
	DDRB |= _BV(DDB5);
}

ISR(PCINT0_vect)
{
    timer_start_ms(10);
}

static
void button_init(void)
{
    DDRB &= ~_BV(DDB4); /* PORTB4 Input */
    PORTB |= _BV(PORTB4); /* Enable pull-up */
    PCICR |= _BV(PCIE0);
    PCMSK0 |= _BV(PCINT4); /* PORTB4 is also PCINT4 */
}

int main (void)
{
    light_init();
    button_init();
    timer_init();
    sei();
    while(1)
    {
    }
	return 0;
}

