#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <winsock2.h>
#include <pthread.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 900
#define GRID_SIZE 30
#define CELL_SIZE (SCREEN_WIDTH / GRID_SIZE)
#define CAR_WIDTH (CELL_SIZE)
#define CAR_HEIGHT (CELL_SIZE)
#define CAR_SPEED 2.0
#define CAR_TURN_SPEED 5
#define PUERTO 12345
#define BUFFERSIZE 1024

#pragma comment(lib, "ws2_32.lib")

typedef struct {
    float x, y;  // posición del carro en la pista
    float angle; // ángulo del carro en grados
} Car;

typedef struct {
    SOCKET sock;
    char mensaje[BUFFERSIZE];
} MessageData;

void *recibir_mensajes(void *socket_desc) {
    SOCKET sock = *(SOCKET *)socket_desc;
    char buffer[BUFFERSIZE];
    int leidos;

    while ((leidos = recv(sock, buffer, BUFFERSIZE, 0)) > 0) {
        buffer[leidos] = '\0';
        printf("Mensaje del servidor: %s\n", buffer);
    }

    if (leidos == 0) {
        printf("El servidor ha cerrado la conexión.\n");
    } else {
        printf("Error en recv(). Código de error: %d\n", WSAGetLastError());
    }

    return NULL;
}

void *enviar_mensajes(void *data) {
    MessageData *messageData = (MessageData *)data;
    while (1) {
        printf("Introduce el mensaje para enviar al servidor: ");
        fgets(messageData->mensaje, BUFFERSIZE, stdin);

        // Enviar mensaje al servidor
        if (send(messageData->sock, messageData->mensaje, strlen(messageData->mensaje), 0) == SOCKET_ERROR) {
            printf("Error al enviar mensaje. Código de error: %d\n", WSAGetLastError());
            break;
        }
    }

    return NULL;
}

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

void drawCar(SDL_Renderer *renderer, SDL_Texture *carTexture, Car *car) {
    SDL_Rect dstRect;
    dstRect.w = CAR_WIDTH;
    dstRect.h = CAR_HEIGHT;
    dstRect.x = (int)car->x - CAR_WIDTH / 2;
    dstRect.y = (int)car->y - CAR_HEIGHT / 2;

    SDL_RenderCopyEx(renderer, carTexture, NULL, &dstRect, car->angle, NULL, SDL_FLIP_NONE);
}

void updateCarPosition(Car *car, int track[GRID_SIZE][GRID_SIZE]) {
    float radianAngle = car->angle * M_PI / 180.0;
    float newX = car->x + cos(radianAngle) * CAR_SPEED;
    float newY = car->y + sin(radianAngle) * CAR_SPEED;

    int cellX = (int)(newX / CELL_SIZE);
    int cellY = (int)(newY / CELL_SIZE);

    if (cellX >= 0 && cellX < GRID_SIZE && cellY >= 0 && cellY < GRID_SIZE && track[cellY][cellX] == 1) {
        car->x = newX;
        car->y = newY;
    }
}

int main(int argc, char* argv[]) {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in servidor_addr;
    pthread_t thread_id, send_thread_id;
    MessageData messageData;

    // Iniciar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Error en WSAStartup. Código de error: %d\n", WSAGetLastError());
        return 1;
    }

    // Crear el socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Error al crear el socket. Código de error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    servidor_addr.sin_family = AF_INET;
    servidor_addr.sin_port = htons(PUERTO);
    servidor_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&servidor_addr, sizeof(servidor_addr)) < 0) {
        printf("Error al conectar con el servidor. Código de error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Conectado al servidor.\n");

    // Crear un hilo para recibir mensajes del servidor
    if (pthread_create(&thread_id, NULL, recibir_mensajes, (void *)&sock) < 0) {
        printf("No se pudo crear el hilo. Código de error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Preparar datos para el hilo de enviar mensajes
    messageData.sock = sock;

    // Crear un hilo para enviar mensajes al servidor
    if (pthread_create(&send_thread_id, NULL, enviar_mensajes, (void *)&messageData) < 0) {
        printf("No se pudo crear el hilo. Código de error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    int map[GRID_SIZE][GRID_SIZE];
    load_track_from_file("adj/track.txt", map);
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *window = SDL_CreateWindow("Pista de Carrera", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface *carSurface = IMG_Load("adj/car1.png");
    if (!carSurface) {
        fprintf(stderr, "IMG_Load Error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *carTexture = SDL_CreateTextureFromSurface(renderer, carSurface);
    SDL_FreeSurface(carSurface);
    if (!carTexture) {
        fprintf(stderr, "SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    Car car = { .x = CELL_SIZE + CELL_SIZE / 2 * 35, .y = CELL_SIZE + CELL_SIZE / 2 * 25, .angle = -90 };

    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_a:
                        car.angle -= CAR_TURN_SPEED;
                        break;
                    case SDLK_d:
                        car.angle += CAR_TURN_SPEED;
                        break;
                }
            }
        }

        updateCarPosition(&car, map);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Color green = {0, 255, 0, 255};
        SDL_Color gray = {128, 128, 128, 255};
        SDL_Color yellow = {246, 226, 29, 255};

        for (int y = 0; y < GRID_SIZE; y++) {
            for (int x = 0; x < GRID_SIZE; x++) {
                if (map[y][x] == 0) {
                    drawCell(renderer, x, y, green);
                } else if (map[y][x] == 1) {
                    drawCell(renderer, x, y, gray);
                    drawBorders(renderer, x, y, map);
                } else if (map[y][x] == 2) {
                    drawCell(renderer, x, y, yellow);
                    drawBorders(renderer, x, y, map);
                }
            }
        }

        drawCar(renderer, carTexture, &car);

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // delay to cap the frame rate at ~60fps
    }

    SDL_DestroyTexture(carTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    // Esperar a que los hilos de recepción y envío terminen
    pthread_join(thread_id, NULL);
    pthread_join(send_thread_id, NULL);

    // Cerrar el socket
    closesocket(sock);
    WSACleanup();

    return 0;
}
