/* Simple library meant to be called from occam that serves as proof
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "occoids_cl.h"

float magnitute2 (vector *ve)
{
  return (ve->x * ve->x) + (ve->y * ve->y);
}

__kernel void occoids (__global agentinfo (*data)[DIM1*DIM2], __global vector *velocity, __global int *aisizes)
{
  // our 2D ids are mapped to x&y
  const int idx = get_global_id(0);
  const int idy = get_global_id(1);
  // combined to give a single id so we can troll through our *data array
  int id = idx + (idy * DIM1);
  int i;
  vector accel;
  accel.x = 0.0;
  accel.y = 0.0;

  // TODO: small rule to do the can.see and make stuff = -1 when it's just there but not visible etc...

  //** centre of mass rule
  vector com;
  com.x = 0.0;
  com.y = 0.0;
  for (i=0; i < aisizes[id]; i++) {
    com.x = com.x + data[id][i].position.x;
    com.y = com.y + data[id][i].position.y;
  }
  // don't do this if there where no agents seen
  if (i > 0) {
    com.x = com.x / i;
    com.y = com.y / i;
  }
  com.x = com.x / CENTREOFMASSFRACT;
  com.y = com.y / CENTREOFMASSFRACT;
  accel.x = com.x + accel.x;
  accel.y = com.y + accel.y;

  //** repulsion rule
  vector push;
  push.x = 0.0;
  push.y = 0.0;
  for (i=0; i < aisizes[id]; i++) {
    // get around address space problems in opencl
    vector pos = data[id][i].position;
    if (magnitute2(&pos) < (REPULSIONDIST * REPULSIONDIST)) {
        push.x = push.x - pos.x;
        push.y = push.y - pos.y;
    }
  }
  push.x = push.x / REPULSIONFRACT;
  push.y = push.y / REPULSIONFRACT;
  accel.x = push.x + accel.x;
  accel.y = push.y + accel.y;

  //** mean velocity rule
  vector pvel;
  pvel.x = 0.0;
  pvel.y = 0.0;
  for (i=0; i < aisizes[id]; i++) {
    pvel.x = pvel.x + data[id][i].velocity.x;
    pvel.y = pvel.y + data[id][i].velocity.y;
  }
  // don't do this if there where no agents seen
  if (i > 0) {
    pvel.x = pvel.x / i;
    pvel.y = pvel.y / i;
  }
  pvel.x = pvel.x - velocity[id].x;
  pvel.y = pvel.y - velocity[id].y;
  pvel.x = pvel.x / MEANVELFRACT;
  pvel.y = pvel.y / MEANVELFRACT;
  accel.x = pvel.x + accel.x;
  accel.y = pvel.y + accel.y;

  //TODO: add obstacle rule

  //** accelerate
  velocity[id].x = velocity[id].x + (accel.x / SMOOTHACCEL);
  velocity[id].y = velocity[id].y + (accel.y / SMOOTHACCEL);

#if 0
  // abs(float x) does not seem to exist in opencl
  if (abs(velocity[id].x) < 0.00000) {
    velocity[id].x = 0.0;
  }
  if (abs(velocity[id].y) < 0.00000) {
    velocity[id].y = 0.0;
  }
#endif

  // get around address space problems in opencl
  vector vel = velocity[id];
  float mag = magnitute2 (&vel);
  if (mag > SPEEDLIMIT2) {
    float div = mag/SPEEDLIMIT2;
    velocity[id].x = velocity[id].x / div;
    velocity[id].y = velocity[id].y / div;
  }
}
