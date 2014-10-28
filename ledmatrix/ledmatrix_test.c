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
#include <util/delay.h>
#include "ledmatrix.h"

#define FRAME_RATE_HZ 50
#define SUBFRAME_RATE_HZ (FRAME_RATE_HZ * N_SUBFRAMES)
#define SUBFRAME_DELAY_US (1000000 / SUBFRAME_RATE_HZ)

int main(void)
{
    struct ledmatrix_frame fB =
        LEDMATRIX_FRAME_INIT(
            11110,
            10001,
            10001,
            11110,
            10001,
            10001,
            11110);

    ledmatrix_setup();

    while(1)
    {
        ledmatrix_draw_next_subframe(&fB);
        _delay_us(SUBFRAME_DELAY_US);
    }
    return 0;
}

