/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include <stdio.h>
#include <string.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>

#include "initial.h"

#define DOUBLE 1
#define MAX_SOURCE_SIZE (0x100000)

#if DOUBLE
typedef double cl_fract;
#else
typedef cl_float cl_fract;
#endif

void _mandelbrot (int *w);
void _initmandelbrot (int *w);
int mandelbrot (cl_char *data, cl_fract *job, cl_int width);
void mandelbrot_c (cl_char *data, cl_fract *job, cl_int width);
int init_mandelbrot ();

