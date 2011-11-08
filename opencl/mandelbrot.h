/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include <stdio.h>
#include <string.h>
#include <complex.h>

#include "initial.h"

void _mandelbrot (int *w);
void _initmandelbrot (int *w);
int mandelbrot (cl_char *data, cl_float *job, cl_int width);
int init_mandelbrot ();

