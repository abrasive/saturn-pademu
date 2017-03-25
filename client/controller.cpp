#include <stdint.h>
#include <vector>
#include <iostream>
#include <string.h>
#include <stdlib.h>

#define COMMON_PAD_CONTROLS                        \
      DigitalControl("Right", report, 1, 7, true), \
      DigitalControl("Left" , report, 1, 6, true), \
      DigitalControl("Down" , report, 1, 5, true), \
      DigitalControl("Up"   , report, 1, 4, true), \
      DigitalControl("Start", report, 1, 3, true), \
      DigitalControl("A"    , report, 1, 2, true), \
      DigitalControl("B"    , report, 1, 0, true), \
      DigitalControl("C"    , report, 1, 1, true), \
      DigitalControl("RT"   , report, 2, 7, true), \
      DigitalControl("X"    , report, 2, 6, true), \
      DigitalControl("Y"    , report, 2, 5, true), \
      DigitalControl("Z"    , report, 2, 4, true), \
      DigitalControl("LT"   , report, 2, 3, true)

class Control {
public:
    Control(const char *name)
        : name(name)
    {}

    const char *name;
};

class DigitalControl : public Control {
    uint8_t *target;
    uint8_t mask;
    bool active_low;

public:
    DigitalControl(const char *name, uint8_t *report, int byte, int bit, bool active_low)
        : Control(name),
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
};

class AnalogControl : public Control {
    uint8_t *target;

public:
    AnalogControl(const char *name, uint8_t *report, int byte)
        : Control(name),
          target(&report[byte])
    {}

    void set(uint8_t value) {
        *target = value;
    }
};

class Peripheral {
protected:
    std::vector<Control> controls{};

    uint8_t *report;

public:
    /* virtual int make_report(uint8_t *buf); */

    Peripheral(int id, int data_size, std::vector<Control> controls)
        : controls(controls)
    {
        report = (uint8_t*)malloc(data_size + 1);
        memset(report, 0, data_size + 1);
        report[0] = (id<<4) | data_size;
    }

    void print_controls(void) {
        std::cout << "controls: " << controls.size() << std::endl;
        for (auto it = controls.begin(); it != controls.end(); ++it) {
            std::cout << it->name << std::endl;
        }
    }
};

class DigitalPad : public Peripheral {
public:
    DigitalPad()
        : Peripheral(0, 2, {
            COMMON_PAD_CONTROLS
          })
    {
        report[1] = report[2] = 0xff;
    }
};

class AnalogPad : public Peripheral {
public:
    AnalogPad()
        : Peripheral(1, 5, {
            COMMON_PAD_CONTROLS,
            AnalogControl("Xaxis", report, 3),
            AnalogControl("Yaxis", report, 4),
            AnalogControl("Zaxis", report, 5),
          })
    {
        report[1] = report[2] = 0xff;
        report[3] = report[4] = report[5] = 0x80;   // midpoint
    }
};

class Keyboard : public Peripheral {
public:
    Keyboard()
        : Peripheral(3, 4, {
            COMMON_PAD_CONTROLS,
            // XXX keyboard control
        })
    {}
};

class Mouse : public Peripheral {
public:
    Mouse()
        : Peripheral(2, 3, {})
    {}
};

class Multitap : public Peripheral {
    Peripheral *port[6];

    Multitap() {
        memset(port, 0, sizeof(port));
    }
};

int main(int argc, char **argv) {
    DigitalPad *pad = new DigitalPad();
    pad->print_controls();
}
