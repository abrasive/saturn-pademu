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

std::multimap<SDL_GameControllerButton, std::pair<SDL_JoystickID, Control*>> InputGamepad::mapping = {};
std::multimap<SDL_GameControllerAxis, std::pair<SDL_JoystickID, Control*>> InputGamepad::axismapping = {};

void InputGamepad::handle_event(SDL_Event *event) {
    if (event->type == SDL_CONTROLLERBUTTONDOWN ||
        event->type == SDL_CONTROLLERBUTTONUP) {
        handle_button_event(event->cbutton);
    } else if (event->type == SDL_CONTROLLERAXISMOTION) {
        handle_axis_event(event->caxis);
    }
}

void InputGamepad::handle_button_event(SDL_ControllerButtonEvent event) {
    bool pressed = event.state == SDL_PRESSED;
    SDL_GameControllerButton button = (SDL_GameControllerButton)event.button;

    if (pressed)
        std::cerr << "Button: " << SDL_GameControllerGetStringForButton(button) << std::endl;

    auto its = mapping.equal_range(button);
    for (auto it = its.first; it != its.second; ++it) {
        SDL_JoystickID id = it->second.first;

        if (id != event.which)
            return;

        DigitalControl* ctl = static_cast<DigitalControl*>(it->second.second);
        ctl->set(pressed);
    }
}

void InputGamepad::handle_axis_event(SDL_ControllerAxisEvent event) {
    SDL_GameControllerAxis axis = (SDL_GameControllerAxis)event.axis;

    auto its = axismapping.equal_range(axis);
    for (auto it = its.first; it != its.second; ++it) {
        SDL_JoystickID id = it->second.first;
        AnalogControl* ctl = static_cast<AnalogControl*>(it->second.second);

        if (id != event.which)
            return;

        Uint16 value = event.value + 32768;
        ctl->set(value / 256);
    }
}

bool InputGamepad::add_mapping(SDL_GameControllerButton inspec, SDL_JoystickID id, Control *ctl) {
    mapping.insert(std::make_pair(inspec, std::make_pair(id, ctl)));

    std::cerr << "mapped key " << SDL_GameControllerGetStringForButton((SDL_GameControllerButton)inspec) << std::endl;

    return true;
}

bool InputGamepad::add_axis_mapping(SDL_GameControllerAxis inspec, SDL_JoystickID id, Control *ctl) {
    axismapping.insert(std::make_pair(inspec, std::make_pair(id, ctl)));

    std::cerr << "mapped axis " << SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)inspec) << std::endl;

    return true;
}
