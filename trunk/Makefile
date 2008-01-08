
PPU_CC_FLAGS=-m64 -Wall
SPU_CC_FLAGS=-Wall
LFLAGS=-lpthread -lspe2 -lSDL
BIN=fractal


all: $(BIN)


$(BIN): main.o image.o spu_fractal_drawer_csf.o
	ppu-gcc $(PPU_CC_FLAGS) -o $(BIN) \
		spu_fractal_drawer_csf.o image.o main.o $(LFLAGS)

##### PPU-MODULIT

main.o: main.c
	ppu-gcc $(PPU_CC_FLAGS) -c main.c

image.o: image.c
	ppu-gcc $(PPU_CC_FLAGS) -c image.c

spu_fractal_drawer_csf.o: spu_fractal_drawer
	embedspu -m64 fractal_handle spu_fractal_drawer spu_fractal_drawer_csf.o


##### SPU-OHJELMA

spu_fractal_drawer: spu_fractal_driver.o mandelbrot.o
	spu-gcc $(SPU_CC_FLAGS) mandelbrot.o spu_fractal_driver.o -o spu_fractal_drawer

##### SPU-MODULIT

spu_fractal_driver.o: spu_fractal_driver.c
	spu-gcc $(SPU_CC_FLAGS) -c spu_fractal_driver.c -o spu_fractal_driver.o

mandelbrot.o: mandelbrot.c
	spu-gcc $(SPU_CC_FLAGS) -c mandelbrot.c -o mandelbrot.o

##### YM.

clean:
	rm -rf *.o $(BIN) spu_fractal_drawer
