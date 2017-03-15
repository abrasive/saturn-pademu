#include <vector>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "peripheral.h"
#include "binding.h"

#define COMMON_PAD_CONTROLS                        \
      controls.push_back(new DigitalControl("Right", report, 1, 7, true)); \
      controls.push_back(new DigitalControl("Left" , report, 1, 6, true)); \
      controls.push_back(new DigitalControl("Down" , report, 1, 5, true)); \
      controls.push_back(new DigitalControl("Up"   , report, 1, 4, true)); \
      controls.push_back(new DigitalControl("Start", report, 1, 3, true)); \
      controls.push_back(new DigitalControl("A"    , report, 1, 2, true)); \
      controls.push_back(new DigitalControl("B"    , report, 1, 1, true)); \
      controls.push_back(new DigitalControl("C"    , report, 1, 0, true)); \
      controls.push_back(new DigitalControl("RT"   , report, 2, 7, true)); \
      controls.push_back(new DigitalControl("X"    , report, 2, 6, true)); \
      controls.push_back(new DigitalControl("Y"    , report, 2, 5, true)); \
      controls.push_back(new DigitalControl("Z"    , report, 2, 4, true)); \
      controls.push_back(new DigitalControl("LT"   , report, 2, 3, true));


Report Peripheral::make_report() {
    Report r;
    r.size = report_size;
    r.oneshot = false;
    memcpy(r.data, report, r.size);
    return r;
}

Peripheral::Peripheral(int id, int data_size)
    : controls({}),
      report_size(data_size+1)
{
    report = (uint8_t*)malloc(report_size);
    memset(report, 0, report_size);
    report[0] = (id<<4) | data_size;
}


void Peripheral::print_controls(void) {
    std::cout << "controls: " << controls.size() << std::endl;
    for (auto it = controls.begin(); it != controls.end(); ++it) {
        std::cout << (*it)->name << std::endl;
    }
}

Control* Peripheral::get_control(const char *name) {
    for (auto it = controls.begin(); it != controls.end(); ++it)
        if (!strcasecmp(name, (*it)->name))
            return *it;
    return NULL;
}

DigitalPad::DigitalPad()
    : Peripheral(0, 2)
{
    COMMON_PAD_CONTROLS;
    report[1] = report[2] = 0xff;
}

AnalogPad::AnalogPad()
    : Peripheral(1, 5)
{
    COMMON_PAD_CONTROLS;
    controls.push_back(new AnalogControl("Xaxis", report, 3));
    controls.push_back(new AnalogControl("Yaxis", report, 4));
    controls.push_back(new AnalogControl("Zaxis", report, 5));

    report[1] = report[2] = 0xff;
    report[3] = report[4] = report[5] = 0x80;   // midpoint
}

// saturn_scancodes: map from Saturn scancodes to SDL keynames
#include "keymap.cpp"

Keyboard::Keyboard()
    : Peripheral(3, 4)
{
    COMMON_PAD_CONTROLS;

    report[1] = 0xff;
    report[2] = 0xf8;   // KBTYPE[2:0] = 000, "SATURN Keyboard"
    report[3] = 6;

    // the keyboard reports certain keys as pad buttons for compatibility reasons
    InputKeyboard::add_mapping("Right", get_control("right"));
    InputKeyboard::add_mapping("Left", get_control("left"));
    InputKeyboard::add_mapping("Up", get_control("up"));
    InputKeyboard::add_mapping("Down", get_control("down"));
    InputKeyboard::add_mapping("Escape", get_control("start"));
    InputKeyboard::add_mapping("Z", get_control("A"));
    InputKeyboard::add_mapping("X", get_control("B"));
    InputKeyboard::add_mapping("C", get_control("C"));
    InputKeyboard::add_mapping("A", get_control("X"));
    InputKeyboard::add_mapping("S", get_control("Y"));
    InputKeyboard::add_mapping("D", get_control("Z"));
    InputKeyboard::add_mapping("E", get_control("LT")); // SMPC manual says this but RT/LT backwards??
    InputKeyboard::add_mapping("Q", get_control("RT"));

    for (auto it = saturn_scancodes.begin(); it != saturn_scancodes.end(); ++it) {
        SDL_Keycode code = SDL_GetKeyFromName(it->second);
        if (!code) {
            std::cerr << "Couldn't find key '" << it->second << "'" << std::endl;
            continue;
        }
        scancode_map.insert(std::make_pair(code, it->first));
    }
}

void Keyboard::sdl_event(SDL_Event *event) {
    if (event->type != SDL_KEYDOWN && event->type != SDL_KEYUP)
        return;

    if (event->key.repeat)
        return;

    bool pressed = event->key.state == SDL_PRESSED;

    static uint8_t test_scancode = 0;
    if (pressed && event->key.keysym.sym == SDL_GetKeyFromName("PageUp")) {
        test_scancode++;
        printf("Scan: %02X\n", test_scancode);
    }
    if (pressed && event->key.keysym.sym == SDL_GetKeyFromName("PageDown")) {
        test_scancode--;
        printf("Scan: %02X\n", test_scancode);
    }

    auto it = scancode_map.find(event->key.keysym.sym);
    if (it == scancode_map.end())
        return;


    scancode = it->second;

    if (event->key.keysym.sym == SDL_GetKeyFromName("X"))
        scancode = test_scancode;


    if (pressed)
        pending = 8;    // MAKE
    else
        pending = 1;    // BREAK
}

Report Keyboard::make_report(void) {
    Report r = Peripheral::make_report();

    if (pending) {
        r.data[3] |= pending;
        r.data[4] = scancode;
        r.oneshot = true;
        pending = 0;
    }

    return r;
}

Mouse::Mouse()
    : Peripheral(2, 3)
{
    controls.push_back(new DigitalControl("Start", report, 1, 3, false));
    controls.push_back(new DigitalControl("Middle", report, 1, 2, false));
    controls.push_back(new DigitalControl("Right", report, 1, 1, false));
    controls.push_back(new DigitalControl("Left" , report, 1, 0, false));

    report[1] = report[2] = report[3] = 0;

    xdist = ydist = 0;
}

void Mouse::sdl_event(SDL_Event *event) {
    if (event->type == SDL_KEYDOWN) {
        SDL_Keymod mods = SDL_GetModState();

        if ((mods & KMOD_CTRL) &&
            (mods & KMOD_ALT)) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            mouse_grabbed = false;
        }
    }
    if (event->type == SDL_MOUSEMOTION) {
        xdist += event->motion.xrel;
        ydist += event->motion.yrel;
    }
    if (event->type == SDL_MOUSEBUTTONDOWN ||
        event->type == SDL_MOUSEBUTTONUP) {
        if (!mouse_grabbed) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            mouse_grabbed = true;
            return;
        }

        bool pressed = event->button.state == SDL_PRESSED;
        switch (event->button.button) {
            case SDL_BUTTON_LEFT:
                static_cast<DigitalControl*>(get_control("Left"))->set(pressed);
                break;
            case SDL_BUTTON_MIDDLE:
                static_cast<DigitalControl*>(get_control("Middle"))->set(pressed);
                break;
            case SDL_BUTTON_RIGHT:
                static_cast<DigitalControl*>(get_control("Right"))->set(pressed);
                break;
        }
    }
}

Report Mouse::make_report(void) {
    Report r = Peripheral::make_report();

    if (!(xdist || ydist))
        return r;

    ydist = -ydist;

    if (xdist < 0)
        r.data[1] |= 0x10;   // X SIGN
    if (ydist < 0)
        r.data[1] |= 0x20;   // Y SIGN

    // the SMPC manual says these should be the absolute value of distance,
    // but at least Jewels of the Oracle expects them to be signed8.
    r.data[2] = xdist & 0xff;
    r.data[3] = ydist & 0xff;

    if (xdist >= 0x100)
        r.data[1] |= 0x40;   // X OVER
    if (ydist >= 0x100)
        r.data[1] |= 0x80;   // Y OVER

    xdist = ydist = 0;
    r.oneshot = true;

    return r;
}


Multitap::Multitap() {
    memset(ports, 0, sizeof(ports));
}

void Multitap::connect(int port, Peripheral *p) {
    ports[port] = p;
}

void Multitap::sdl_event(SDL_Event *event) {
    for (int i=0; i<6; i++)
        if (ports[i])
            ports[i]->sdl_event(event);
}

Report Multitap::make_report(void) {
    Report r;
    r.oneshot = false;

    uint8_t *p = r.data;

    // The SMPC datasheet says we should say 0x01, 0x60 here. that's a lie.
    *p++ = 0x40;
    *p++ = 0x60;

    for (int i=0; i<6; i++) {
        if (!ports[i]) {
            *p++ = 0xff;
            r.size += 1;
        } else {
            Report pr = ports[i]->make_report();
            memcpy(p, pr.data, pr.size);
            p += pr.size;
            if (pr.oneshot)
                r.oneshot = true;
        }
    }

    r.size = p - r.data;

    return r;
}
