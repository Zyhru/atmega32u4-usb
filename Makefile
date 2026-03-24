OBJS = build/main.hex build/main.elf

all:
	avr-gcc -mmcu=atmega32u4 -Wall -I/usr/avr/include -Os -o build/main.elf main.c 
	avr-objcopy -j .text -j .data -O ihex build/main.elf build/main.hex

clean:
	rm $(OBJS)
