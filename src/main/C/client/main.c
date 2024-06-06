#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <winsock2.h>
#include <pthread.h>
#include <windows.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 900
#define GRID_SIZE 30
#define CELL_SIZE (SCREEN_WIDTH / GRID_SIZE)
#define CAR_WIDTH (CELL_SIZE)
#define CAR_HEIGHT (CELL_SIZE)
#define CAR_SPEED 3.0
#define CAR_TURN_SPEED 5
#define PUERTO 12345
#define BUFFERSIZE 1024
#define ARDUINO_PORT "COM4" // Puerto serie donde está conectado Arduino
#define ARDUINO_BAUDRATE CBR_9600 // Velocidad de baudios de Arduino

#pragma comment(lib, "ws2_32.lib")

typedef struct {
    float x, y;  // posición del carro en la pista
    float angle; // ángulo del carro en grados
} Car;

typedef struct {
    float speed;
    float turn_speed;
} CarSpeed;

typedef struct {
    SOCKET sock;
    HANDLE hSerial; // Añade esta línea para el puerto serie
    char mensaje[BUFFERSIZE];
    int track[GRID_SIZE][GRID_SIZE];
    CarSpeed carSpeed; // Añadir la estructura de velocidad del carro
    Car *car; // Puntero al carro
} MessageData;

void *recibir_mensajes(void *data) {
    MessageData *messageData = (MessageData *)data;
    SOCKET sock = messageData->sock;
    char buffer[BUFFERSIZE];
    int leidos;

    while ((leidos = recv(sock, buffer, BUFFERSIZE, 0)) > 0) {
        buffer[leidos] = '\0';
        printf("Mensaje del servidor: %s\n", buffer);

        int x, y, valor;
        if (sscanf(buffer, "%d,%d,%d", &x, &y, &valor) == 3) {
            if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
                messageData->track[y][x] = valor;
            }
        } else {
            char car[4];
            float new_speed;
            if (sscanf(buffer, "%3s,%f", car, &new_speed) == 2) {
                if (strcmp(car, "car") == 0) {
                    messageData->carSpeed.speed = new_speed;
                }
            }
        }
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
    char mensaje[BUFFERSIZE];

    while (1) {
        printf("Introduce el mensaje para enviar al servidor: ");
        fgets(mensaje, BUFFERSIZE, stdin);

        // Enviar mensaje al servidor
        if (send(messageData->sock, mensaje, strlen(mensaje), 0) == SOCKET_ERROR) {
            printf("Error al enviar mensaje. Código de error: %d\n", WSAGetLastError());
            break;
        }
    }

    return NULL;
}

void *enviar_posicion(void *data) {
    MessageData *messageData = (MessageData *)data;
    char mensaje[BUFFERSIZE];

    while (1) {
        snprintf(mensaje, BUFFERSIZE, "pos,%f,%f,%f", messageData->car->x, messageData->car->y, messageData->car->angle);

        // Enviar mensaje al servidor
        if (send(messageData->sock, mensaje, strlen(mensaje), 0) == SOCKET_ERROR) {
            printf("Error al enviar mensaje de posición. Código de error: %d\n", WSAGetLastError());
            break;
        }

        SDL_Delay(1000); // Enviar la posición cada 1 segundo
    }

    return NULL;
}
    
void *leer_desde_arduino(void *data) {
    MessageData *messageData = (MessageData *)data;
    char buffer[BUFFERSIZE];
    DWORD bytesRead;

    while (1) {
        // Leer datos desde Arduino
        if (ReadFile(messageData->hSerial, buffer, sizeof(buffer), &bytesRead, NULL)) {
            buffer[bytesRead] = '\0'; // Agregar el carácter nulo al final
            if (bytesRead > 0) {
                printf(buffer);
                // Aquí puedes procesar los datos recibidos desde Arduino según sea necesario
            }
        }
        Sleep(100); // Retardo de 100ms
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

void updateCarPosition(Car *car, int track[GRID_SIZE][GRID_SIZE], float car_speed) {
    float radianAngle = car->angle * M_PI / 180.0;
    float newX = car->x + cos(radianAngle) * car_speed;
    float newY = car->y + sin(radianAngle) * car_speed;

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
    pthread_t thread_id, send_thread_id, pos_thread_id, arduino_thread_id;
    Car car = { .x = (CELL_SIZE*19), .y = (CELL_SIZE*13), .angle = -90 };
    MessageData messageData = { .carSpeed = { CAR_SPEED, CAR_TURN_SPEED }, .car = &car };

    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Error en WSAStartup(). Código de error: %d\n", WSAGetLastError());
        return 1;
    }

    // Crear socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Error al crear el socket. Código de error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    servidor_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servidor_addr.sin_family = AF_INET;
    servidor_addr.sin_port = htons(PUERTO);

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&servidor_addr, sizeof(servidor_addr)) < 0) {
        printf("Error al conectar con el servidor. Código de error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Conectado al servidor.\n");

    messageData.sock = sock;

    // Crear el hilo para recibir mensajes del servidor
    if (pthread_create(&thread_id, NULL, recibir_mensajes, (void *)&messageData) < 0) {
        printf("Error al crear el hilo para recibir mensajes.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Crear el hilo para enviar mensajes al servidor
    if (pthread_create(&send_thread_id, NULL, enviar_mensajes, (void *)&messageData) < 0) {
        printf("Error al crear el hilo para enviar mensajes.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Crear el hilo para enviar la posición del carro al servidor
    if (pthread_create(&pos_thread_id, NULL, enviar_posicion, (void *)&messageData) < 0) {
        printf("Error al crear el hilo para enviar la posición.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Abrir el puerto serie para comunicarse con Arduino
    messageData.hSerial = CreateFile(ARDUINO_PORT, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (messageData.hSerial == INVALID_HANDLE_VALUE) {
        printf("Error al abrir el puerto serie para Arduino.\n");
        return 1;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(messageData.hSerial, &dcbSerialParams)) {
        printf("Error al obtener el estado del puerto serie de Arduino.\n");
        CloseHandle(messageData.hSerial);
        return 1;
    }

    dcbSerialParams.BaudRate = ARDUINO_BAUDRATE;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(messageData.hSerial, &dcbSerialParams)) {
        printf("Error al configurar el estado del puerto serie de Arduino.\n");
        CloseHandle(messageData.hSerial);
        return 1;
    }

    // Crear el hilo para leer desde Arduino
    if (pthread_create(&arduino_thread_id, NULL, leer_desde_arduino, (void *)&messageData) < 0) {
        printf("Error al crear el hilo para leer desde Arduino.\n");
        CloseHandle(messageData.hSerial);
        return 1;
    }

    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL no pudo inicializarse. SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Car Simulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("La ventana no pudo crearse. SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("El renderer no pudo crearse. SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return 1;
    }

    SDL_Surface* carSurface = IMG_Load("adj/car1.png");
    if (!carSurface) {
        printf("No se pudo cargar la imagen del carro. IMG_Error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    SDL_Texture* carTexture = SDL_CreateTextureFromSurface(renderer, carSurface);
    SDL_FreeSurface(carSurface);
    if (!carTexture) {
        printf("No se pudo crear la textura del carro. SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    load_track_from_file("adj/track.txt", messageData.track);

    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_a:
                        car.angle -= messageData.carSpeed.turn_speed;
                        break;
                    case SDLK_d:
                        car.angle += messageData.carSpeed.turn_speed;
                        break;
                    case SDLK_w:
                        if (messageData.carSpeed.speed < 5){
                            messageData.carSpeed.speed +=  1.0;
                        }
                        break;
                    case SDLK_s:
                        if (messageData.carSpeed.speed > 2){
                            messageData.carSpeed.speed +=  -1.0;
                        }
                        break;
                }
            }
        }

        updateCarPosition(&car, messageData.track, messageData.carSpeed.speed);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Color green = {0, 255, 0, 255};
        SDL_Color gray = {128, 128, 128, 255};
        SDL_Color yellow = {246, 226, 29, 255};

        for (int y = 0; y < GRID_SIZE; y++) {
            for (int x = 0; x < GRID_SIZE; x++) {
                if (messageData.track[y][x] == 0) {
                    drawCell(renderer, x, y, green);
                } else if (messageData.track[y][x] == 1) {
                    drawCell(renderer, x, y, gray);
                    drawBorders(renderer, x, y, messageData.track);
                } else if (messageData.track[y][x] == 2) {
                    drawCell(renderer, x, y, yellow);
                    drawBorders(renderer, x, y, messageData.track);
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
    SDL_Quit();

    // Cerrar el socket y limpiar Winsock
    closesocket(sock);
    WSACleanup();
    // Cerrar el puerto serie de Arduino
    CloseHandle(messageData.hSerial);

    return 0;
}
