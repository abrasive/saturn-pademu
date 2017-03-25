LIBS=`sdl2-config --libs`
CFLAGS=`sdl2-config --cflags` -g -O2
CXXFLAGS=$(CFLAGS) -std=c++11

OS := $(shell uname -s)

ifeq ($(OS), Darwin)
LDFLAGS=$(LIBS) -framework CoreFoundation -framework IOKit
hid_target=hid_MACOSX.c
else ifeq ($(OS), Linux)
LDFLAGS=$(LIBS) -lusb
hid_target=hid_LINUX.c
else ifeq ($(OS), WINDOWS)
LDFLAGS=$(LIBS) -lhid -lsetupapi
hid_target=hid_WINDOWS.c
# ¯\_(ツ)_/¯
else
LDFLAGS=$(LIBS)
endif

.PHONY: keymap

client: client.cpp peripheral.cpp binding.cpp peripheral.h binding.h keymap.cpp hid.o usb.cpp
	g++ $(CXXFLAGS) usb.cpp peripheral.cpp binding.cpp client.cpp hid.o -o client $(LDFLAGS)

hid.o: rawhid/$(hid_target)
	cc $(CFLAGS) -c $< -o $@

keymap.cpp: keymap genkeymap.py
	python genkeymap.py > keymap.cpp

clean:
	rm -f client keymap.cpp *.o
	rm -rf client.dSYM
