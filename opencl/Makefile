CC=gcc
CFLAGS=-O3 -march=native -Wall -lOpenCL -lm -I.
DEPS = 
OBJ = hellocl.o hellocl_test.o initial.o rot13.o sin.o mandelbrot.o printf.o modulo.o occoids.o colour.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

hellocl: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
clean:
	rm -f hellocl.o hellocl_test.o initial.o rot13.o sin.o mandelbrot.o printf.o modulo.o occoids.o colour.o hellocl
