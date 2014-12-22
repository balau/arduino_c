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
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>

#define BVV(bit, val) ((val)?_BV(bit):0)
#define DUTY_MAX 100

static void pwm_set_duty(int8_t duty)
{
    uint8_t oc;
    if (duty <= 0)
    {
        oc = 0;
    }
    else if (duty >= DUTY_MAX)
    {
        oc = 0xFF;
    }
    else
    {
        oc = (duty * (int16_t)0xFF) / DUTY_MAX;
    }
    OCR0A = oc;
}

static void pwm_init(int8_t duty)
{
	DDRD |= _BV(DDD6); /* set pin 6 of PORTD for output*/
    TCCR0A = 
          BVV(WGM00, 1) | BVV(WGM01, 1) /* Fast PWM update on OCRA */
        | BVV(COM0A1, 1) | BVV(COM0A0, 0) /* non-inverting OC0A */
        | BVV(COM0B1, 0) | BVV(COM0B0, 0); /* OC0B not connected */
    pwm_set_duty(duty);
    TCCR0B =
          BVV(CS00, 1) | BVV(CS01, 0) | BVV(CS02, 1) /* F_CPU/1024 */
        | BVV(WGM02, 0) /* Fast PWM update on OCRA */
        | BVV(FOC0A, 0) | BVV(FOC0B, 0); /* ignored */
}

int main (void)
{
    int8_t duty = 0;
    bool rising = true;

    pwm_init(duty);

	while(true) {
        if(rising)
        {
            duty++;
            if(duty >= DUTY_MAX)
            {
                rising = false;
            }
        }
        else
        {
            duty--;
            if(duty <= 0)
            {
                rising = true;
            }
        }
        pwm_set_duty(duty);
        _delay_ms(10);
	}
}

