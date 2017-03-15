#include <SDL2/SDL.h>
#include <iostream>
#include "peripheral.h"
#include "binding.h"

bool Input::add_mapping(const char *inspec, Control *ctl) {
    if (InputKeyboard::add_mapping(inspec, ctl))
        return true;

    std::cerr << "Could not find input " << inspec << " to map" << std::endl;
    return false;
}

void Input::handle_event(SDL_Event *event) {
    InputKeyboard::handle_event(event);
}

std::multimap<SDL_Keycode, DigitalControl*> InputKeyboard::mapping = {};

void InputKeyboard::handle_event(SDL_Event *event) {
    if (event->type != SDL_KEYDOWN &&
        event->type != SDL_KEYUP)
        return;

    if (event->key.repeat)
        return;

    bool pressed = event->key.state == SDL_PRESSED;

    if (pressed)
        std::cerr << "Key: " << SDL_GetKeyName(event->key.keysym.sym) << std::endl;

    auto its = mapping.equal_range(event->key.keysym.sym);
    for (auto it = its.first; it != its.second; ++it)
        it->second->set(pressed);
}

bool InputKeyboard::add_mapping(const char *inspec, Control *ctl) {
    SDL_Keycode code = SDL_GetKeyFromName(inspec);
    if (code == SDLK_UNKNOWN)
        return false;

    if (ctl->type != Digital) {
        std::cerr << ctl->name << " is not a digital control, can't map to keyboard" << std::endl;
        std::cerr << ctl->type << std::endl;
        return false;
    }

    mapping.insert(std::make_pair(code, static_cast<DigitalControl*>(ctl)));

    std::cerr << "mapped key " << inspec << std::endl;

    return true;
}
