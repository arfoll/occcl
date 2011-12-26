/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "sin.h"
#include "rot13.h"
#include "mandelbrot.h"
#include "modulo.h"

int getCorrectDevice(char *requiredExt);
int mandelbrotTest();
int moduloTest();
int sinTest();
int rot13Test();
int occoidsTest();
