all: build_hex flash_write
build_hex: sdcc_build_hex
flash_write: avrdude_flash_write
flash_verify: avrdude_flash_verify
flash_erase: avrdude_flash_erase

# Instruct the compiler ("-I .") to look for current folder's hd44780_pinbus.h file - see header declaration in ../src/hd44780_pinbus.c
sdcc_build_hex: main_lcd1602.c
	sdcc -I . -c ../src/hd44780_pinbus.c
	sdar -rc hd44780_pinbus.lib hd44780_pinbus.rel
	sdcc main_lcd1602.c hd44780_pinbus.lib -L hd44780_pinbus.lib
	packihx main_lcd1602.ihx > main_lcd1602.hex

# Change here the USB port of your setup.
usb_port = /dev/ttyUSB0
avrdude_flash_write: main_lcd1602.hex
	avrdude -C ../conf/AT89S5x.conf -c stk500v1 -P $(usb_port) -p 89s51 -b 19200 -U flash:w:'$(PWD)/main_lcd1602.hex'

avrdude_flash_verify: main_lcd1602.hex
	avrdude -C ../conf/AT89S5x.conf -c stk500v1 -P $(usb_port) -p 89s51 -b 19200 -U flash:v:'$(PWD)/main_lcd1602.hex'

avrdude_flash_erase:
	avrdude -C ../conf/AT89S5x.conf -c stk500v1 -P $(usb_port) -p 89s51 -b 19200 -e

clean:
	rm -f *.lnk
	rm -f *.ihx
	rm -f *.lst
	rm -f *.map
	rm -f *.rel
	rm -f *.rst
	rm -f *.sym
	rm -f *.asm
	rm -f *.lk
	rm -f *.mem
	rm -f *.lib