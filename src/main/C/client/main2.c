#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <pthread.h>

#define PUERTO 12345
#define BUFFERSIZE 1024

#pragma comment(lib, "ws2_32.lib")

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

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in servidor_addr;
    char mensaje[BUFFERSIZE];
    pthread_t thread_id;

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

    // Enviar mensajes al servidor
    while (1) {
        printf("Introduce el mensaje para enviar al servidor: ");
        fgets(mensaje, BUFFERSIZE, stdin);

        // Enviar mensaje al servidor
        if (send(sock, mensaje, strlen(mensaje), 0) == SOCKET_ERROR) {
            printf("Error al enviar mensaje. Código de error: %d\n", WSAGetLastError());
            break;
        }
    }

    // Esperar a que el hilo de recepción termine
    pthread_join(thread_id, NULL);

    // Cerrar el socket
    closesocket(sock);
    WSACleanup();

    return 0;
}
