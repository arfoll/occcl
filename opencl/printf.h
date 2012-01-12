/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#ifndef PRINTF_H
#define PRINTF_H

#include "initial.h"
#include <stdio.h>

void _printfjob (int *w);
void _printfreal (int *w);
void _printfint (int *w);

#endif
