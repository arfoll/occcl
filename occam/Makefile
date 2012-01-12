CC=kroc
KROCOPT = --cc-opts=-lOpenCL
DEPS = 
OBJ = ../opencl/initial.o
OBJ_HELLO = ../opencl/rot13.o ../opencl/mandelbrot.o ../opencl/colour.o ../opencl/occoids.o
OBJ_MAN = ../opencl/mandelbrot.o ../opencl/printf.o ../opencl/colour.o
OBJ_OCCOIDS = ../opencl/occoids.o ../opencl/printf.o

build:
	$(CC) hello.occ $(KROCOPT) ${OBJ} ${OBJ_HELLO}
	$(CC) mandelbrot.occ $(KROCOPT) ${OBJ} ${OBJ_MAN}
	$(CC) mandelbrot-bench.occ $(KROCOPT) ${OBJ} ${OBJ_MAN}
	$(CC) mandelbrot-occam.occ
	$(CC) occoids-single.occ $(KROCOPT) ${OBJ} ${OBJ_OCCOIDS}

clean:
	rm -f hello occoids-single mandelbrot mandelbrot-bench
