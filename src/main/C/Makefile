# Directorios de inclusión y biblioteca
INCLUDE_DIR := include
LIB_DIR := lib

# Fuentes
SRCS := client/main2.c client/draw.c client/game.c client/math.c

# Objetivo principal
all: main

# Comando para construir el ejecutable principal
main: $(SRCS)
	gcc -I $(INCLUDE_DIR) -I $(INCLUDE_DIR)/SDL2 -L $(LIB_DIR) -o main4 $(SRCS) -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf

# Comando para limpiar archivos generados
clean:
	rm -f client/*.o main
