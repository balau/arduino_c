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
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define T1_MAX 0xFFFFUL
#define T1_PRESCALER 1024
#define T1_TICK_US (T1_PRESCALER/(F_CPU/1000000UL)) /* 64us @ 16MHz */
#define T1_MAX_US (T1_TICK_US * T1_MAX) /* ~4.2s @ 16MHz */

static void led_on(void)
{
    PORTB |= _BV(PORTB5);
}

static void led_off(void)
{
    PORTB &= ~_BV(PORTB5);
}

static void led_init(void)
{
	DDRB |= _BV(DDB5); /* PORTB5 as output */
    led_off();
}

static void timer_stop(void)
{
    TCCR1B &= ~(_BV(CS10)|_BV(CS11)|_BV(CS12)); /* stop timer clock */
    TIMSK1 &= ~_BV(TOIE1); /* disable interrupt */
    TIFR1 |= _BV(TOV1); /* clear interrupt flag */
}

static void timer_init(void)
{
    /* normal mode */
    TCCR1A &= ~(_BV(WGM10)|_BV(WGM11));
    TCCR1B &= ~(_BV(WGM13)|_BV(WGM12));
    timer_stop();
}

static void timer_start(unsigned long us)
{
    unsigned long ticks_long;
    unsigned short ticks;

    ticks_long = us / T1_TICK_US;
    if (ticks_long >= T1_MAX)
    {
        ticks = T1_MAX;
    }
    else
    {
        ticks = ticks_long;
    }
    TCNT1 = T1_MAX - ticks; /* overflow in ticks*1024 clock cycles */

    TIMSK1 |= _BV(TOIE1); /* enable overflow interrupt */
    /* start timer clock */
    TCCR1B &= ~(_BV(CS10)|_BV(CS11)|_BV(CS12));
    TCCR1B |= _BV(CS10)|_BV(CS12); /* prescaler: 1024 */
}

static void timer_start_ms(unsigned short ms)
{
    timer_start(ms * 1000UL);
}

ISR(TIMER1_OVF_vect) /* timer 1 interrupt service routine */
{
    timer_stop();
    led_off(); /* timeout expired: turn off LED */
}

ISR(PCINT0_vect) /* pin change interrupt service routine */
{
    led_on();
    timer_stop();
    if (bit_is_set(PINB, PINB4)) /* button released */
    {
        timer_start_ms(2000); /* timeout to turn off LED */
    }
}

static void button_init(void)
{
    DDRB &= ~_BV(DDB4); /* PORTB4 as input */
    PORTB |= _BV(PORTB4); /* enable pull-up */
    PCICR |= _BV(PCIE0); /* enable Pin Change 0 interrupt */
    PCMSK0 |= _BV(PCINT4); /* PORTB4 is also PCINT4 */
}

int main (void)
{
    led_init();
    button_init();
    timer_init();
    sei(); /* enable interrupts globally */
    while(true)
    {
        sleep_mode();
    }
}

