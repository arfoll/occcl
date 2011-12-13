CC=gcc
CFLAGS=-g3 -Wall -lOpenCL -lm -I.
DEPS = 
OBJ = hellocl.o hellocl_test.o initial.o rot13.o sin.o mandelbrot.o printf.o modulo.o occoids.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

hellocl: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)
clean:
	rm -f hellocl.o hellocl_test.o initial.o rot13.o sin.o mandelbrot.o printf.o modulo.o occoids.o hellocl
