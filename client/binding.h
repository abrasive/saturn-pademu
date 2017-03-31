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

class InputGamepad : public Input {
    static std::multimap<SDL_GameControllerButton, std::pair<SDL_JoystickID, Control*>> mapping;
    static std::multimap<SDL_GameControllerAxis, std::pair<SDL_JoystickID, Control*>> axismapping;
public:
    static bool add_mapping(SDL_GameControllerButton inspec, SDL_JoystickID id, Control *ctl);
    static bool add_axis_mapping(SDL_GameControllerAxis inspec, SDL_JoystickID id, Control *ctl);
    static void handle_event(SDL_Event *event);
    static void handle_button_event(SDL_ControllerButtonEvent event);
    static void handle_axis_event(SDL_ControllerAxisEvent event);
};
