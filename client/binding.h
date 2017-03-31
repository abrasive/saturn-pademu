#include "peripheral.h"
#include <map>

class Input {
public:
    static bool add_mapping(const char *inspec, Control *ctl);
    static void handle_event(SDL_Event *event);
};

class InputKeyboard : public Input {
    static std::multimap<SDL_Keycode, Control*> mapping;
public:
    static bool add_mapping(const char *inspec, Control *ctl);
    static void handle_event(SDL_Event *event);
};

struct JoyAxisMapping {
    SDL_JoystickID joy;
    Control *ctl;
    int sign;   // ±1, multiplied with value
    bool onesided;  // whether to scale input from 0-32727 rather than -32728-32767

    // hysteresis on and off points for digital controls
    int hyst_on, hyst_off;
};

class InputGamepad : public Input {
    static std::multimap<SDL_GameControllerButton, std::pair<SDL_JoystickID, Control*>> mapping;
    static std::multimap<SDL_GameControllerAxis, struct JoyAxisMapping> axismapping;
public:
    static bool add_mapping(SDL_GameControllerButton inspec, SDL_JoystickID id, Control *ctl);
    static bool add_axis_mapping(SDL_GameControllerAxis inspec, SDL_JoystickID id, Control *ctl,
                                 int sign=1, int hyst_on=0x90*128, int hyst_off=0x56*128);
    static void handle_event(SDL_Event *event);
    static void handle_button_event(SDL_ControllerButtonEvent event);
    static void handle_axis_event(SDL_ControllerAxisEvent event);
};
