/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#ifndef ROT13_H
#define ROT13_H

#include <stdio.h>
#include <string.h>

#include "initial.h"

void _init_rot13 (int *w);
void _rot13 (int *w);
int rot13 (char* plaintext, char *ciphertext);
int init_rot13 ();

#endif
