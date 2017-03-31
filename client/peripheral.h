#include <stdint.h>
#include <vector>
#include <map>
#include <SDL2/SDL.h>
#pragma once

enum ControlType {
    None,
    Analog,
    Digital,
};

class Control {
public:
    Control(const char *name, enum ControlType type)
        : name(name),
          type(type)
    {
    }

    const char *name;

    const enum ControlType type;

    virtual void set(bool value) = 0;
    virtual void set(int16_t value) = 0;
};

class DigitalControl : public Control {
public: // XXX
    uint8_t *target;
    uint8_t mask;
    bool active_low;

public:
    DigitalControl(const char *name, uint8_t *report, int byte, int bit, bool active_low)
        : Control(name, Digital),
          target(&report[byte]),
          mask(1<<bit),
          active_low(active_low)
    {}

    void set(bool value) {
        if (active_low)
            value = !value;

        if (value)
            *target |= mask;
        else
            *target &= ~mask;
    }

    // map analog values (eg. triggers) to digital with hysteresis
    void set(int16_t value) {
        if (value > 0x9000)
            set(true);
        if (value < 0x5600)
            set(false);
    }
};

class AnalogControl : public Control {
    uint8_t *target;
    bool onesided;

public:
    AnalogControl(const char *name, bool onesided, uint8_t *report, int byte)
        : Control(name, Analog),
          target(&report[byte]),
          onesided(onesided)
    {}

    void set(int16_t value) {
        uint8_t scaled;
        if (onesided) {
            if (value < 0)
                value = 0;
            scaled = value / 128;
        } else {
            scaled = ((int)value + 32768) / 256;
        }
        *target = scaled;
    }

    void set(bool value) {
        if (value)
            set((int16_t)INT16_MAX);
        else
            set((int16_t)INT16_MIN);
    }
};

struct Report {
    int size;
    bool oneshot;
    uint8_t data[127];
};

class Peripheral {
protected:
    std::vector<Control*> controls;

    uint8_t *report;
    int report_size;

public:
    // make_report() must be called once after each event loop
    virtual struct Report make_report();

    Peripheral(int id, int data_size);
    Peripheral() : report_size(0) {};   // unplugged (or custom)

    void print_controls(void);

    Control * get_control(const char *name);

    virtual void sdl_event(SDL_Event *event) {};
};

class DigitalPad : public Peripheral {
public:
    DigitalPad();

    void sdl_event(SDL_Event *event) {};
};

class AnalogPad : public Peripheral {
public:
    AnalogPad();
};

class Keyboard : public Peripheral {
    uint8_t scancode;
    uint8_t pending;
    std::map<SDL_Keycode, uint8_t> scancode_map;
public:
    Keyboard();
    void sdl_event(SDL_Event *event);
    struct Report make_report();
};

class Mouse : public Peripheral {
    int xdist, ydist;
    bool mouse_grabbed;
public:
    Mouse();
    void sdl_event(SDL_Event *event);
    struct Report make_report();
};

class Multitap : public Peripheral {
    Peripheral *ports[6];

public:
    Multitap();

    void connect(int port, Peripheral *p);

    void sdl_event(SDL_Event *event);
    struct Report make_report();
};
