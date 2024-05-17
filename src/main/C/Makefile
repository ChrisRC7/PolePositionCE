SRCS := $(wildcard client/*.c)
OBJS := $(SRCS:.c=.o)

all: main

main: $(OBJS)
		gcc -o $@ $^ -I include -L lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf

clean:
		rm -f $(OBJS) main

# Generar dependencias
depend: $(SRCS)
	gcc -M $(SRCS) -I include/SDL2 > depend

# Incluir dependencias en el Makefile
include depend

# Compilar archivos de origen
%.o: %.c
		gcc -c -o $@ $< -I include
