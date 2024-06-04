#include "./draw.h"
#include <SDL2/SDL.h>
#include <math.h>

const int BACKGROUND_R = 0;
const int BACKGROUND_G = 0;
const int BACKGROUND_B = 0;

const int LINE_SPACING = 30;
const int LINE_COUNT = 20;

void draw(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, BACKGROUND_R, BACKGROUND_G, BACKGROUND_B, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < LINE_COUNT; i++) {
        float scale = 1.0f - (i / (float)LINE_COUNT);
        int y = HEIGHT - (i * LINE_SPACING);
        int lineWidth = WIDTH * scale;

        // Dibujar línea central
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, (WIDTH - lineWidth) / 2, y, (WIDTH + lineWidth) / 2, y);

        // Dibujar bordes rojos y blancos
        SDL_SetRenderDrawColor(renderer, (i % 2 == 0) ? 255 : 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, (WIDTH - lineWidth) / 2, y, (WIDTH - lineWidth) / 2 + 10, y);
        SDL_RenderDrawLine(renderer, (WIDTH + lineWidth) / 2, y, (WIDTH + lineWidth) / 2 - 10, y);
    }
    SDL_RenderPresent(renderer);
}
