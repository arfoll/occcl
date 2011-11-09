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

#define SINGLE 1

#if SINGLE
typedef cl_float cl_fract;
#else
typedef double cl_fract;
#endif

void _mandelbrot (int *w);
void _initmandelbrot (int *w);
int mandelbrot (cl_char *data, cl_fract *job, cl_int width, cl_fract *y);
int init_mandelbrot ();

