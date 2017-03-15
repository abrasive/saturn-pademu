LDFLAGS=`sdl2-config --libs` -lusb
CFLAGS=`sdl2-config --cflags` -g -O2
CXXFLAGS=$(CFLAGS) -std=c++11

client: client.cpp peripheral.cpp binding.cpp peripheral.h binding.h keymap.cpp hid.o usb.cpp
	g++ $(CXXFLAGS) usb.cpp peripheral.cpp binding.cpp client.cpp hid.o -o client $(LDFLAGS)

hid.o: rawhid/hid_LINUX.c
	cc $(CFLAGS) -c $< -o $@

keymap.cpp: keymap genkeymap.py
	python genkeymap.py > keymap.cpp

clean:
	rm -f client
