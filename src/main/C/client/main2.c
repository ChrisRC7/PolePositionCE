#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 900
#define GRID_SIZE 30
#define CELL_SIZE (SCREEN_WIDTH / GRID_SIZE)


void load_track_from_file(const char *filename, int track[GRID_SIZE][GRID_SIZE]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            if (fscanf(file, "%d", &track[i][j]) != 1) {
                fprintf(stderr, "Error reading file at line %d, column %d\n", i + 1, j + 1);
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(file);
}


void drawCell(SDL_Renderer *renderer, int x, int y, SDL_Color color) {
    SDL_Rect rect = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void drawBorders(SDL_Renderer *renderer, int x, int y, int map[GRID_SIZE][GRID_SIZE]) {
    SDL_Rect rect;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color red = {255, 0, 0, 255};

    if (y > 0 && map[y-1][x] == 0) { // top
        rect = (SDL_Rect){ x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE / 6 };
        SDL_SetRenderDrawColor(renderer, white.r, white.g, white.b, white.a);
        SDL_RenderFillRect(renderer, &rect);
        rect.y += CELL_SIZE / 6;
        SDL_SetRenderDrawColor(renderer, red.r, red.g, red.b, red.a);
        SDL_RenderFillRect(renderer, &rect);
    }
    if (y < GRID_SIZE - 1 && map[y+1][x] == 0) { // bottom
        rect = (SDL_Rect){ x * CELL_SIZE, (y + 1) * CELL_SIZE - CELL_SIZE / 6, CELL_SIZE, CELL_SIZE / 6 };
        SDL_SetRenderDrawColor(renderer, red.r, red.g, red.b, red.a);
        SDL_RenderFillRect(renderer, &rect);
        rect.y -= CELL_SIZE / 6;
        SDL_SetRenderDrawColor(renderer, white.r, white.g, white.b, white.a);
        SDL_RenderFillRect(renderer, &rect);
    }
    if (x > 0 && map[y][x-1] == 0) { // left
        rect = (SDL_Rect){ x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE / 6, CELL_SIZE };
        SDL_SetRenderDrawColor(renderer, white.r, white.g, white.b, white.a);
        SDL_RenderFillRect(renderer, &rect);
        rect.x += CELL_SIZE / 6;
        SDL_SetRenderDrawColor(renderer, red.r, red.g, red.b, red.a);
        SDL_RenderFillRect(renderer, &rect);
    }
    if (x < GRID_SIZE - 1 && map[y][x+1] == 0) { // right
        rect = (SDL_Rect){ (x + 1) * CELL_SIZE - CELL_SIZE / 6, y * CELL_SIZE, CELL_SIZE / 6, CELL_SIZE };
        SDL_SetRenderDrawColor(renderer, red.r, red.g, red.b, red.a);
        SDL_RenderFillRect(renderer, &rect);
        rect.x -= CELL_SIZE / 6;
        SDL_SetRenderDrawColor(renderer, white.r, white.g, white.b, white.a);
        SDL_RenderFillRect(renderer, &rect);
    }
}

int main(int argc, char* argv[]) {
    int map[GRID_SIZE][GRID_SIZE];
    load_track_from_file("adj/track.txt", map);
    SDL_Init(SDL_INIT_VIDEO);
    

    SDL_Window *window = SDL_CreateWindow("Pista de Carrera", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Color green = {0, 255, 0, 255};
        SDL_Color gray = {128, 128, 128, 255};
        SDL_Color yelloy = {246, 226, 29, 255};

        for (int y = 0; y < GRID_SIZE; y++) {
            for (int x = 0; x < GRID_SIZE; x++) {
                if (map[y][x] == 0) {
                    drawCell(renderer, x, y, green);
                } else if (map[y][x] == 1) {
                    drawCell(renderer, x, y, gray);
                    drawBorders(renderer, x, y, map);
                } else if (map[y][x]== 2) {
                    drawCell(renderer, x, y, yelloy);
                    drawBorders(renderer, x, y, map);
                }
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
