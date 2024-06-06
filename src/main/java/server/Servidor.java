package server;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import java.util.Arrays;
import java.util.Scanner;

public class Servidor {
    private static final int GRID_SIZE = 30;
    private static int[][] track = new int[GRID_SIZE][GRID_SIZE];
    private static JFrame frame;
    private static TrackPanel trackPanel;
    private static int selectedValue = 2; // Valor inicial del botón (hueco)
    private static Socket socket;
    private static PrintWriter salida;

    public static void main(String[] args) {
        int puerto = 12345; // El puerto en el que el servidor escuchará
        cargarTrack("src\\main\\C\\adj\\track.txt"); // Cargar la matriz de track

        // Crear la interfaz gráfica
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                createAndShowGUI();
            }
        });

        try (ServerSocket servidor = new ServerSocket(puerto)) {
            System.out.println("Servidor iniciado. Esperando conexiones...");

            while (true) {
                socket = servidor.accept();
                salida = new PrintWriter(socket.getOutputStream(), true);
                System.out.println("Cliente conectado: " + socket.getInetAddress().getHostAddress());

                // Hilos para manejo de la comunicación
                new Thread(new ClienteHandler(socket)).start();
                new Thread(new ConsoleHandler(socket)).start();
            }
        } catch (IOException e) {
            System.err.println("Error al iniciar el servidor: " + e.getMessage());
        }
    }

    // Método para cargar el track desde un archivo
    private static void cargarTrack(String filePath) {
        try (Scanner scanner = new Scanner(new File(filePath))) {
            for (int i = 0; i < GRID_SIZE; i++) {
                for (int j = 0; j < GRID_SIZE; j++) {
                    if (scanner.hasNextInt()) {
                        track[i][j] = scanner.nextInt();
                    } else {
                        throw new IOException("El archivo de track no tiene suficientes datos");
                    }
                }
            }
        } catch (FileNotFoundException e) {
            System.err.println("Archivo de track no encontrado: " + e.getMessage());
        } catch (IOException e) {
            System.err.println("Error al leer el archivo de track: " + e.getMessage());
        }
    }

    // Método sincronizado para actualizar el track
    public synchronized static void updateTrack(int x, int y, int value) {
        if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
            track[y][x] = value;
            System.out.println("Track actualizado en [" + y + ", " + x + "] con valor " + value);
            trackPanel.repaint(); // Redibujar la pista
        }
    }

    // Método sincronizado para obtener el valor del track
    public synchronized static int getTrackValue(int x, int y) {
        if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
            return track[y][x];
        }
        return -1; // Valor inválido
    }

    private static void createAndShowGUI() {
        frame = new JFrame("Pista");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setLayout(new BorderLayout());

        // Panel para la pista
        trackPanel = new TrackPanel();
        frame.add(trackPanel, BorderLayout.CENTER);

        // Panel para los botones
        JPanel buttonPanel = new JPanel();
        JButton huecoButton = new JButton("Hueco");
        JButton velocidadButton = new JButton("Velocidad");
        JButton vidaButton = new JButton("Vida");

        huecoButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                selectedValue = 2;
            }
        });
        velocidadButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                selectedValue = 3;
            }
        });
        vidaButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                selectedValue = 4;
            }
        });

        buttonPanel.add(huecoButton);
        buttonPanel.add(velocidadButton);
        buttonPanel.add(vidaButton);

        frame.add(buttonPanel, BorderLayout.SOUTH);

        frame.setSize(900, 900);
        frame.setVisible(true);
    }

    static class TrackPanel extends JPanel {
        @Override
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            int cellSize = getWidth() / GRID_SIZE;
            for (int i = 0; i < GRID_SIZE; i++) {
                for (int j = 0; j < GRID_SIZE; j++) {
                    if (track[i][j] == 0) {
                        g.setColor(Color.GREEN);
                    } else if (track[i][j] == 1) {
                        g.setColor(Color.GRAY);
                    } else if (track[i][j] == 2) {
                        g.setColor(Color.YELLOW);
                    } else if (track[i][j] == 3) {
                        g.setColor(Color.BLUE);
                    } else if (track[i][j] == 4) {
                        g.setColor(Color.RED);
                    }
                    g.fillRect(j * cellSize, i * cellSize, cellSize, cellSize);
                    g.setColor(Color.BLACK);
                    g.drawRect(j * cellSize, i * cellSize, cellSize, cellSize);
                }
            }

            // Añadir el listener para manejar los clics
            addMouseListener(new MouseAdapter() {
                @Override
                public void mouseClicked(MouseEvent e) {
                    int cellSize = getWidth() / GRID_SIZE;
                    int x = e.getX() / cellSize;
                    int y = e.getY() / cellSize;

                    if (socket != null && salida != null) {
                        salida.println(x + "," + y + "," + selectedValue);
                        Servidor.updateTrack(x, y, selectedValue);
                    } else {
                        System.err.println("No hay conexión con el servidor.");
                    }
                }
            });
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
        try (BufferedReader entrada = new BufferedReader(new InputStreamReader(socket.getInputStream()));
             PrintWriter salida = new PrintWriter(socket.getOutputStream(), true)) {
            String mensaje;
            System.out.println("ClienteHandler iniciado para: " + socket.getInetAddress().getHostAddress());

            while ((mensaje = entrada.readLine()) != null) {
                System.out.println("Mensaje del cliente: " + mensaje);

                if (mensaje.startsWith("pos,")) {
                    // Mensaje de posición del vehículo
                    String[] partes = mensaje.split(",");
                    System.out.println("Partes del mensaje: " + Arrays.toString(partes));

                    if (partes.length == 4) {
                        try {
                            float x = Float.parseFloat(partes[1]);
                            float y = Float.parseFloat(partes[2]);
                            float angle = Float.parseFloat(partes[3]);

                            int cellX = (int) (x / (900 / 30));
                            int cellY = (int) (y / (900 / 30));

                            if (cellX >= 0 && cellX < 30 && cellY >= 0 && cellY < 30) {
                                if (Servidor.getTrackValue(cellX, cellY) == 2) {
                                    salida.println("car,1");
                                    salida.println(cellX + "," + cellY + ",1");
                                    Servidor.updateTrack(cellX, cellY, 1); // Actualizar el track
                                }
                            }
                        } catch (NumberFormatException e) {
                            System.err.println("Error al parsear la posición: " + e.getMessage());
                        }
                    }
                }
            }
        } catch (IOException e) {
            System.err.println("Error al leer del cliente: " + e.getMessage());
        } finally {
            try {
                socket.close();
                System.out.println("Conexión con el cliente cerrada.");
            } catch (IOException e) {
                System.err.println("Error al cerrar el socket: " + e.getMessage());
            }
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
            System.out.println("ConsoleHandler iniciado para: " + socket.getInetAddress().getHostAddress());

            while ((mensaje = consola.readLine()) != null) {
                salida.println(mensaje);
                System.out.println("Mensaje enviado al cliente: " + mensaje);

                // Actualizar la matriz track si el mensaje es del tipo x,y,value
                String[] partes = mensaje.split(",");
                if (partes.length == 3) {
                    try {
                        int x = Integer.parseInt(partes[0]);
                        int y = Integer.parseInt(partes[1]);
                        int value = Integer.parseInt(partes[2]);
                        Servidor.updateTrack(x, y, value); // Actualizar el track
                    } catch (NumberFormatException e) {
                        System.err.println("Error al parsear el mensaje de la consola: " + e.getMessage());
                    }
                }
            }
        } catch (IOException e) {
            System.err.println("Error al escribir al cliente: " + e.getMessage());
        }
    }
}
