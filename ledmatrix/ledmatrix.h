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

#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include <stdint.h>

/* Using LTP-7357AG
 * 7x5 LED matrix
 */

#define N_COLS 5
#define N_ROWS 7

struct ledmatrix_frame {
    uint8_t cols[N_COLS];
};

#define BITCONST_(bits) 0b ## bits

#define GETCOLDOT_(i_col, i_row, row) (((BITCONST_(row) >> (N_COLS - 1 - (i_col))) & 0x01) << (i_row))

#define GETCOL_(i_col, r0,r1,r2,r3,r4,r5,r6) \
    ( \
      GETCOLDOT_(i_col, 0, r0)| \
      GETCOLDOT_(i_col, 1, r1)| \
      GETCOLDOT_(i_col, 2, r2)| \
      GETCOLDOT_(i_col, 3, r3)| \
      GETCOLDOT_(i_col, 4, r4)| \
      GETCOLDOT_(i_col, 5, r5)| \
      GETCOLDOT_(i_col, 6, r6)  \
    )

#define FRAME(r0,r1,r2,r3,r4,r5,r6) \
    {{ \
        GETCOL_(0, r0,r1,r2,r3,r4,r5,r6), \
        GETCOL_(1, r0,r1,r2,r3,r4,r5,r6), \
        GETCOL_(2, r0,r1,r2,r3,r4,r5,r6), \
        GETCOL_(3, r0,r1,r2,r3,r4,r5,r6), \
        GETCOL_(4, r0,r1,r2,r3,r4,r5,r6), \
    }}

extern void ledmatrix_setup(void);

extern void ledmatrix_draw_col(int i_col, uint8_t dots);

extern void ledmatrix_draw_frame_col(const struct ledmatrix_frame *f, int i_col);

extern void ledmatrix_draw_next_col(const struct ledmatrix_frame *f);

#endif /* LEDMATRIX_H */

