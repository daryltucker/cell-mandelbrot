
PPU_CC_FLAGS=-m64 -Wall
SPU_CC_FLAGS=-Wall
LFLAGS=-lpthread -lspe2 -lX11
BIN=mandelbrot


all: $(BIN)


$(BIN): main.o image.o spe_fractal_csf.o
	ppu-gcc $(PPU_CC_FLAGS) -o $(BIN) spe_fractal_csf.o image.o main.o $(LFLAGS)

main.o: main.c
	ppu-gcc $(PPU_CC_FLAGS) -c main.c

image.o: image.c
	ppu-gcc $(PPU_CC_FLAGS) -c image.c

spe_fractal_csf.o: spe_fractal
	embedspu -m64 fractal_handle spe_fractal spe_fractal_csf.o

spe_fractal: spu_fractal_driver.c
	spu-gcc $(SPU_CC_FLAGS) spu_fractal_driver.c -o spe_fractal


clean:
	rm -rf *.o $(BIN)