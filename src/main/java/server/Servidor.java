package server;

import java.io.*;
import java.net.*;

public class Servidor {
    /**
     * The main function sets up a server socket on a specified port, accepts client connections, and
     * creates separate threads to handle communication with clients and console input.
     */
    public static void main(String[] args) {
        int puerto = 12345; // El puerto en el que el servidor escuchará
        try (ServerSocket servidor = new ServerSocket(puerto)) {
            System.out.println("Servidor iniciado. Esperando conexiones...");

            while (true) {
                Socket socket = servidor.accept();
                System.out.println("Cliente conectado: " + socket.getInetAddress().getHostAddress());

                // Hilos para manejo de la comunicación
                new Thread(new ClienteHandler(socket)).start();
                new Thread(new ConsoleHandler(socket)).start();
            }
        } catch (IOException e) {
            System.err.println("Error al iniciar el servidor: " + e.getMessage());
        }
    }
}

// Clase para manejar mensajes desde el cliente
class ClienteHandler implements Runnable {
    private Socket socket;

    public ClienteHandler(Socket socket) {
        this.socket = socket;
    }

    @Override
    public void run() {
        try (BufferedReader entrada = new BufferedReader(new InputStreamReader(socket.getInputStream()))) {
            String mensaje;
            while ((mensaje = entrada.readLine()) != null) {
                System.out.println("Mensaje del cliente: " + mensaje);
            }
        } catch (IOException e) {
            System.err.println("Error al leer del cliente: " + e.getMessage());
        }
    }
}

// Clase para manejar entradas de la consola
class ConsoleHandler implements Runnable {
    private Socket socket;

    public ConsoleHandler(Socket socket) {
        this.socket = socket;
    }

    @Override
    public void run() {
        try (PrintWriter salida = new PrintWriter(socket.getOutputStream(), true);
             BufferedReader consola = new BufferedReader(new InputStreamReader(System.in))) {
            String mensaje;
            while ((mensaje = consola.readLine()) != null) {
                salida.println(mensaje);
                System.out.println("Mensaje enviado al cliente: " + mensaje);
            }
        } catch (IOException e) {
            System.err.println("Error al escribir al cliente: " + e.getMessage());
        }
    }
}
