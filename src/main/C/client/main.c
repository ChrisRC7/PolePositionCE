#include <stdio.h>
#include <SDL2/SDL.h>

// Dimensiones de la ventana
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* args[]) {
    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL no se pudo inicializar. SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Crear ventana
    SDL_Window* window = SDL_CreateWindow("Mi Juego", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("La ventana no se pudo crear. SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Obtener el renderizador
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("No se pudo crear el renderizador. SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Color de fondo
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Limpiar la pantalla
    SDL_RenderClear(renderer);

    // Dibujar un rectángulo rojo en el centro de la pantalla
    SDL_Rect rect = {SCREEN_WIDTH/2 - 50, SCREEN_HEIGHT/2 - 50, 100, 100};
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);

    // Actualizar la pantalla
    SDL_RenderPresent(renderer);

    // Esperar 3 segundos antes de cerrar la ventana
    SDL_Delay(3000);

    // Liberar recursos y cerrar SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
