/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#ifndef SIN_H
#define SIN_H

#include <stdio.h>
#include <string.h>

#include "initial.h"

#define NUM_GPUS 1

int clsin (cl_float *data);
int init_sin ();

#endif
