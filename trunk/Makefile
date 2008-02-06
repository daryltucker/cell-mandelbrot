
PPU_CC_FLAGS=-g -m64 -Wall
SPU_CC_FLAGS=-g -Wall
LFLAGS=-lpthread -lspe2 -lSDL


all: c_fractal asm_fractal


c_fractal: main.o image.o spu_fractal_drawer_csf.o
	ppu-gcc $(PPU_CC_FLAGS) -o c_fractal \
		spu_fractal_drawer_csf.o image.o main.o $(LFLAGS)

asm_fractal: main.o image.o spu_asm_fractal_drawer_csf.o
	ppu-gcc $(PPU_CC_FLAGS) -o asm_fractal \
		spu_asm_fractal_drawer_csf.o image.o main.o $(LFLAGS)


##### PPU-MODULIT

main.o: main.c
	ppu-gcc $(PPU_CC_FLAGS) -c main.c

image.o: image.c
	ppu-gcc $(PPU_CC_FLAGS) -c image.c

spu_fractal_drawer_csf.o: spu_fractal_drawer
	embedspu -m64 fractal_handle spu_fractal_drawer spu_fractal_drawer_csf.o

spu_asm_fractal_drawer_csf.o: spu_asm_fractal_drawer
	embedspu -m64 fractal_handle spu_asm_fractal_drawer spu_asm_fractal_drawer_csf.o


##### SPU-OHJELMA

spu_fractal_drawer: spu_fractal_driver.o mandelbrot.o
	spu-gcc $(SPU_CC_FLAGS) mandelbrot.o spu_fractal_driver.o -o spu_fractal_drawer

spu_asm_fractal_drawer: spu_fractal_driver.o asm_mandelbrot.o
	spu-gcc $(SPU_CC_FLAGS) asm_mandelbrot.o spu_fractal_driver.o -o spu_asm_fractal_drawer

##### SPU-MODULIT

spu_fractal_driver.o: spu_fractal_driver.c
	spu-gcc $(SPU_CC_FLAGS) -c spu_fractal_driver.c -o spu_fractal_driver.o

mandelbrot.o: mandelbrot.c
	spu-gcc $(SPU_CC_FLAGS) -c mandelbrot.c -o mandelbrot.o

asm_mandelbrot.o: mandelbrot.s
	spu-gcc $(SPU_CC_FLAGS) -c mandelbrot.s -o asm_mandelbrot.o

##### YM.

clean:
	rm -rf *.o c_fractal asm_fractal spu_fractal_drawer spu_asm_fractal_drawer
