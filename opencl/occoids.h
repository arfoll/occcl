/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#ifndef OCCOIDS_H
#define OCCOIDS_H

#include <stdio.h>
#include <stdlib.h>

#include "initial.h"

#define MAX_SOURCE_SIZE (0x100000)

#define CLOCCOIDS 0

struct vector {
  float x;
  float y;
};

struct agentinfo {
  int localid;
  int type;
  struct vector position;
  struct vector velocity;
  float radius;
  int colour;
};

void _occoids (int *w);
void _initoccoids (int *w);
int occoids (struct agentinfo ai, struct vector accel);
int occoids_c (struct agentinfo ai, struct vector accel);
int init_occoids ();
cl_int print_occoids_kernel_info ();

#endif
