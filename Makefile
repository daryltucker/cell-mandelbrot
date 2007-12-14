
SPU_CC_FLAGS=-Wall
BIN=cell-mandelbrot


all: $(BIN)

$(BIN):
	ppu-gcc -o $(BIN) main.o

main.o: main.c
	ppu-gcc $(SPU_CC_FLAGS) -c main.c

clean:
	rm -rf *.o $(BIN)
