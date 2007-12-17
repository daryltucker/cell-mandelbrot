
SPU_CC_FLAGS=-m64 -Wall
LFLAGS=-lpthread -lspe2
BIN=cell-mandelbrot


all: $(BIN)

$(BIN): main.o
	ppu-gcc -o $(BIN) main.o

main.o: main.c
	ppu-gcc $(SPU_CC_FLAGS) -c main.c $(LFLAGS)

clean:
	rm -rf *.o $(BIN)
