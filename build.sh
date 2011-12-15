#!/bin/bash

cd opencl
#make clean
make

cd ..
#occbuild --prog hello.occ --include hello.inc -lOpenCL ../opencl/initial.o ../opencl/rot13.o
# hello.occ
#kroc -I/usr/share/kroc/vtlib -L/usr/share/kroc/vtlib -I/usr/share/kroc/vtinclude -L/usr/share/kroc/vtinclude -I/home/brendan/git/occcl/occam -L/home/brendan/git/occcl/occam -I/home/brendan/git/occcl/opencl -L/home/brendan/git/occcl/opencl -o occam/hello occam/hello.occ --cc-opts=-lOpenCL opencl/initial.o opencl/rot13.o opencl/mandelbrot.o

#mandelbrot.occ
kroc -d -I/usr/share/kroc/vtlib -L/usr/share/kroc/vtlib -I/usr/share/kroc/vtinclude -L/usr/share/kroc/vtinclude -I/home/brendan/git/occcl/occam -L/home/brendan/git/occcl/occam -I/home/brendan/git/occcl/opencl -L/home/brendan/git/occcl/opencl -o occam/mandelbrot occam/mandelbrot.occ --cc-opts=-lOpenCL opencl/initial.o opencl/mandelbrot.o opencl/printf.o opencl/colour.o

#occoids-single.occ
#kroc -I/usr/share/kroc/vtlib -L/usr/share/kroc/vtlib -I/usr/share/kroc/vtinclude -L/usr/share/kroc/vtinclude -I/home/brendan/git/occcl/occam -L/home/brendan/git/occcl/occam -I/home/brendan/git/occcl/opencl -L/home/brendan/git/occcl/opencl -o occam/occoids-single occam/occoids-single.occ --cc-opts=-lOpenCL opencl/initial.o opencl/occoids.o opencl/printf.o

