
OBJS	= build/main.hex build/main.elf
MCU		= atmega32u4
F_CPU   = 16000000UL
F_USB   = $(F_CPU)

CFLAGS  = -mmcu=$(MCU) \
           -DF_CPU=$(F_CPU) \
           -DF_USB=$(F_USB) \
           -DARCH=ARCH_AVR8 \
           -fshort-enums \
           -funsigned-char \
           -funsigned-bitfields \
           -fno-strict-aliasing \
           -ffunction-sections \
           -std=gnu99 \
           -Wall \
           -Os

all:
	avr-gcc $(CFLAGS) -o build/main.elf main.c usb.c
	avr-objcopy -j .text -j .data -O ihex build/main.elf build/main.hex

clean:
	rm $(OBJS)
