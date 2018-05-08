os.elf: com.c pipe.c mem.c test.c
	avr-gcc -mmcu=atmega644p -DF_CPU=12000000 -Wall -Wextra -Os $^ -o $@
os.hex: os.elf
	avr-objcopy -O ihex $< $@
install: os.hex
	avrdude -c usbasp -p m644p -U flash:w:os.hex
