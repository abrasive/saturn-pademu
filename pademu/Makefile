ARD=~/pademu/arduino-1.8.1

build/pademu.ino.hex: pademu.ino
	mkdir -p build
	$(ARD)/arduino-builder -hardware $(ARD)/hardware -tools $(ARD)/hardware/tools/avr -tools $(ARD)/tools-builder -libraries $(ARD)/libraries -fqbn arduino:avr:leonardo -build-path `pwd`/build -warnings all $<

clean:
	rm -rf build

up: build/pademu.ino.hex
	stty 1200 hupcl < /dev/ttyACM0
	sleep 1
	avrdude -qqq -patmega32u4 -cavr109 -P/dev/ttyACM0 -b57600 -D -V -Uflash:w:$<:i
