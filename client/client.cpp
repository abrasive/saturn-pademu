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

    SDL_Window *win = SDL_CreateWindow("Saturn Pad Client", 100, 100, 320, 200, SDL_WINDOW_SHOWN);
    if (!win)
        sdl_die("SDL_CreateWindow failed");

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren)
        sdl_die("SDL_CreateRenderer failed");

    /* Keyboard *p = new Keyboard(); */
    /* Mouse *p = new Mouse(); */
    /* Input::add_mapping("Space", p->get_control("start")); */
    Peripheral *p1 = new DigitalPad();
    Input::add_mapping("Right", p1->get_control("right"));
    Input::add_mapping("Left", p1->get_control("left"));
    Input::add_mapping("Up", p1->get_control("up"));
    Input::add_mapping("Down", p1->get_control("down"));
    Input::add_mapping("Escape", p1->get_control("start"));
    Input::add_mapping("Z", p1->get_control("A"));
    Input::add_mapping("X", p1->get_control("B"));
    Input::add_mapping("C", p1->get_control("C"));
    Input::add_mapping("A", p1->get_control("X"));
    Input::add_mapping("S", p1->get_control("Y"));
    Input::add_mapping("D", p1->get_control("Z"));
    Input::add_mapping("E", p1->get_control("LT")); // SMPC manual says this but RT/LT backwards??
    Input::add_mapping("Q", p1->get_control("RT"));

    Peripheral *p2 = new DigitalPad();
    Input::add_mapping("S", p2->get_control("right"));
    Input::add_mapping("A", p2->get_control("left"));
    Input::add_mapping("W", p2->get_control("up"));
    Input::add_mapping("R", p2->get_control("down"));

    /* Peripheral *p = p1; */
    /* Input::add_mapping("Right", p->get_control("right")); */
    /* Input::add_mapping("Left", p->get_control("left")); */
    /* Input::add_mapping("Up", p->get_control("up")); */
    /* Input::add_mapping("Down", p->get_control("down")); */
    /* Input::add_mapping("Escape", p->get_control("start")); */
    /* Input::add_mapping("Z", p->get_control("A")); */
    /* Input::add_mapping("X", p->get_control("B")); */
    /* Input::add_mapping("C", p->get_control("C")); */
    /* Input::add_mapping("A", p->get_control("X")); */
    /* Input::add_mapping("S", p->get_control("Y")); */
    /* Input::add_mapping("D", p->get_control("Z")); */
    /* Input::add_mapping("E", p->get_control("LT")); // SMPC manual says this but RT/LT backwards?? */
    /* Input::add_mapping("Q", p->get_control("RT")); */

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

        Input::handle_event(&ev);
        p1->sdl_event(&ev);
        p2->sdl_event(&ev);

        {
        Report r = p1->make_report();
        for (int i=0; i<r.size; i++)
            printf(" %02x", r.data[i]);
        printf("\n");
        usb->send_report(0, r);
        }
        
        {
        Report r = p2->make_report();
        for (int i=0; i<r.size; i++)
            printf(" %02x", r.data[i]);
        printf("\n");
        usb->send_report(1, r);
        }
    }
}
