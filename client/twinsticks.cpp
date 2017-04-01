#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "binding.h"
#include "peripheral.h"
#include "usb.h"

void sdl_die(const char *msg) {
    printf("ERROR: %s: %s\n", msg, SDL_GetError());
    exit(1);
}


int main(int argc, char **argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER))
        sdl_die("SDL_Init failed");

    SDL_Window *win = SDL_CreateWindow("Saturn Twin Stick Client", 100, 100, 320, 200, SDL_WINDOW_SHOWN);
    if (!win)
        sdl_die("SDL_CreateWindow failed");

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren)
        sdl_die("SDL_CreateRenderer failed");

    if (!SDL_IsGameController(0)) {
        sdl_die("No controller 0 found");
    }

    SDL_GameController *pad = SDL_GameControllerOpen(0);
    if (!pad)
        sdl_die("Couldn't open controller 0");
    SDL_JoystickID ctl = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pad));
    Peripheral *p1 = new TwinStick();
    InputGamepad::add_axis_mapping(SDL_CONTROLLER_AXIS_LEFTX, ctl,
                                   p1->get_control("Lleft"), -1);
    InputGamepad::add_axis_mapping(SDL_CONTROLLER_AXIS_LEFTX, ctl,
                                   p1->get_control("Lright"), 1);
    InputGamepad::add_axis_mapping(SDL_CONTROLLER_AXIS_LEFTY, ctl,
                                   p1->get_control("Lup"), -1);
    InputGamepad::add_axis_mapping(SDL_CONTROLLER_AXIS_LEFTY, ctl,
                                   p1->get_control("Ldown"), 1);
    InputGamepad::add_mapping(SDL_CONTROLLER_BUTTON_LEFTSHOULDER, ctl,
                              p1->get_control("Lbutton"));
    InputGamepad::add_axis_mapping(SDL_CONTROLLER_AXIS_TRIGGERLEFT, ctl,
                                   p1->get_control("Ltrigger"));
    InputGamepad::add_axis_mapping(SDL_CONTROLLER_AXIS_RIGHTX, ctl,
                                   p1->get_control("Rleft"), -1);
    InputGamepad::add_axis_mapping(SDL_CONTROLLER_AXIS_RIGHTX, ctl,
                                   p1->get_control("Rright"), 1);
    InputGamepad::add_axis_mapping(SDL_CONTROLLER_AXIS_RIGHTY, ctl,
                                   p1->get_control("Rup"), -1);
    InputGamepad::add_axis_mapping(SDL_CONTROLLER_AXIS_RIGHTY, ctl,
                                   p1->get_control("Rdown"), 1);
    InputGamepad::add_mapping(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, ctl,
                              p1->get_control("Rbutton"));
    InputGamepad::add_axis_mapping(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, ctl,
                                   p1->get_control("Rtrigger"));
    InputGamepad::add_mapping(SDL_CONTROLLER_BUTTON_START, ctl,
                              p1->get_control("start"));

    Padulator *usb = new Padulator();

    SDL_Event ev;
    while (SDL_WaitEvent(&ev)) {
        if (ev.type == SDL_QUIT)
            break;

        if (ev.type == SDL_WINDOWEVENT &&
            ev.window.event == SDL_WINDOWEVENT_EXPOSED) {
            SDL_RenderClear(ren);
            SDL_RenderPresent(ren);
            continue;
        }

        InputGamepad::handle_event(&ev);
        p1->sdl_event(&ev);

        {
        Report r = p1->make_report();
        for (int i=0; i<r.size; i++)
            printf(" %02x", r.data[i]);
        printf("\n");
        usb->send_report(0, r);
        }
    }
}
