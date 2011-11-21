/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include <stdlib.h>
#include <string.h>
#include "initial.h"

int init_modulo ();
int modulo (cl_int *num, cl_float *mod, cl_float *amount);
