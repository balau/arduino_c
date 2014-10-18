#
# Copyright (c) 2014 Francesco Balducci
#
# This file is part of arduino_c.
#
#    arduino_c is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    embedded-snippets is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with embedded-snippets.  If not, see <http://www.gnu.org/licenses/>.
#

ifeq (${PROG},)
$(error Please specify PROG = ... in your main Makefile.)
endif

ifeq (${SRC},)
$(error Please specify SRC += ... in your main Makefile.)
endif

# First target is default target.
all:

AVRDUDE=avrdude
CC=avr-gcc
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump

MCU=atmega328p
F_CPU=16000000UL

PORT=/dev/ttyACM0
BAUD=115200
PROTOCOL=arduino
PART=ATMEGA328P

AVRDUDEFLAGS= -F -V -c ${PROTOCOL} -p ${PART} -P ${PORT} -b ${BAUD}
CFLAGS += -g
CPPFLAGS += -DF_CPU=${F_CPU}
TARGET_ARCH = -mmcu=${MCU}
COPTFLAG = -Os
CFLAGS += ${COPTFLAG}
#LDFLAGS += -Xlinker -Map=$(PROG).map 

SRC_C = $(filter %.c,${SRC})
SRC_s = $(filter %.s,${SRC})
SRC_S = $(filter %.S,${SRC})

OBJ = $(SRC_C:.c=.o) $(SRC_s:.s=.o) $(SRC_S:.S=.o)

.PHONY: all clean upload download

%.hex: %
	${OBJCOPY} -O ihex -R .eeprom $< $@

%.bin: %
	${OBJCOPY} -O binary -R .eeprom $< $@

%.code: %
	${OBJDUMP} -S $< >$@

%.lst: %
	${OBJDUMP} -d $< >$@

${PROG}: ${OBJ}

all: ${PROG} ${PROG}.hex

clean:
	rm -f ${PROG} $(addprefix ${PROG}, .hex .map .code .lst .bin) ${OBJ}

upload: ${PROG}.hex
	${AVRDUDE} ${AVRDUDEFLAGS} -U flash:w:$<

download:
	${AVRDUDE} ${AVRDUDEFLAGS} -U flash:r:flash_`date +%Y%m%d%H%M%S`.hex:i

