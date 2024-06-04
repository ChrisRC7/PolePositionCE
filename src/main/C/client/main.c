#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "./draw.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer;
SDL_Event e;
bool quit = false;

void init() {
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("Pista de Carreras", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

void gameLoop() {
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            quit = true;
        }
    }
}

int main(int arg, char *argv[]) {
    init();

    while (!quit) {
        gameLoop();
        draw(renderer);
        SDL_Delay(16); // Aproximadamente 60 fps
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
