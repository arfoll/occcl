/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "initial.h"

#define MAX_SOURCE_SIZE (0x100000)
#define IMAGEWIDTH 100
#define IMAGEHEIGHT 50
#define NFRAMES 850

#define DEBUG 0
#define CLMANDEL 1
#define C_PRINT 0 
#define DOUBLE 1

#if DOUBLE
typedef double cl_fract;
#else
typedef cl_float cl_fract;
#endif

#if C_PRINT
#include "colour.h"
#endif

void _mandelbrot (int *w);
void _initmandelbrot (int *w);
int mandelbrot (cl_char (*data)[50][200]);
void mandelbrot_c (cl_char (*data)[200], cl_fract *job);
int init_mandelbrot ();
cl_int print_mandelbrot_kernel_info ();

#endif
