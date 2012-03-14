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
#define NUM_GPUS 1

#define CLOCCOIDS 1
#define DEBUG 0

#define ATBOID 1
#define ATCYLINDER 2
#define CENTREOFMASSFRACT 45.0f
#define REPULSIONDIST 0.06f
#define REPULSIONFRACT 4.0f
#define MEANVELFRACT 8.0f
#define OBSTACLEFRACT 8.0f
#define SOFTTHRESHOLD 0.05f
#define SMOOTHACCEL 5.0f
#define SPEEDLIMIT 0.03f
#define SPEEDLIMIT2 (SPEEDLIMIT * SPEEDLIMIT) //0.0009
#define VISIONRADIUS 0.25f
#define VISIONANGLE 200.0f

typedef struct {
  cl_float x;
  cl_float y;
} vector;

typedef struct {
  cl_int localid;
  cl_int type;
  vector position;
  vector velocity;
  cl_float radius;
  cl_int colour;
} agentinfo;

void _occoids (int *w);
void _initoccoids (int *w);
int occoids (agentinfo *ai, vector *velocity, cl_int *size, cl_int *aisizes);
int occoids_c (agentinfo *ai, vector *velocity, cl_int size);
int init_occoids ();
cl_int print_occoids_kernel_info ();

#endif
