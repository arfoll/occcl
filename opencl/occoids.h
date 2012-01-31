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
#include <math.h>

#include "initial.h"

#define MAX_SOURCE_SIZE (0x100000)
#define CLOCCOIDS 0
#define DEBUG 0

#define ATBOID 1
#define ATCYLINDER 2
#define CENTREOFMASSFRACT 45.0
#define REPULSIONDIST 0.06
#define REPULSIONFRACT 4.0
#define MEANVELFRACT 8.0
#define OBSTACLEFRACT 8.0
#define SOFTTHRESHOLD 0.05
#define SPEEDLIMIT 0.03
#define SMOOTHACCEL 5.0
#define SPEEDLIMIT2 SPEEDLIMIT * SPEEDLIMIT
#define VISIONRADIUS 0.25
#define VISIONANGLE 200.0

typedef struct {
  cl_float x;
  cl_float y;
} vector;

typedef struct {
  int localid;
  int type;
  vector position;
  vector velocity;
  cl_float radius;
  int colour;
} agentinfo;

void _occoids (int *w);
void _initoccoids (int *w);
int occoids (agentinfo *ai, vector *velocity, int size);
int occoids_c (agentinfo *ai, vector *velocity, int size);
int init_occoids ();
cl_int print_occoids_kernel_info ();

#endif
