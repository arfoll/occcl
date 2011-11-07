#!/bin/bash

cd opencl
#make clean
make

cd ../occam
#occbuild --prog hello.occ --include hello.inc -lOpenCL ../opencl/initial.o ../opencl/rot13.o
kroc -I/usr/share/kroc/vtlib -L/usr/share/kroc/vtlib -I/usr/share/kroc/vtinclude -L/usr/share/kroc/vtinclude -I/home/brendan/git/occcl/occam -L/home/brendan/git/occcl/occam -I/home/brendan/git/occcl/opencl -L/home/brendan/git/occcl/opencl -o hello hello.occ --cc-opts=-lOpenCL ../opencl/initial.o ../opencl/rot13.o
