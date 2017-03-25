#include "peripheral.h"
#include <map>

class Input {
public:
    static bool add_mapping(const char *inspec, Control *ctl);
    static void handle_event(SDL_Event *event);
};

class InputKeyboard : public Input {
    static std::multimap<SDL_Keycode, DigitalControl*> mapping;
public:
    static bool add_mapping(const char *inspec, Control *ctl);
    static void handle_event(SDL_Event *event);
};

class InputGamepad : public Input {
    static std::multimap<SDL_GameControllerButton, DigitalControl*> mapping;
public:
    static bool add_mapping(SDL_GameControllerButton inspec, Control *ctl);
    static void handle_event(SDL_Event *event);
};
