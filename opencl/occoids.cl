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
  const int id = idx + (idy * DIM1);

  // Vars we work with. To reduce memory usage we reuse variables violently
  int i;
  vector temp;
  vector accel;

  // TODO: small rule to do the can.see and make stuff = -1 when it's just there but not visible etc...

  //** centre of mass rule
  temp.x = 0.0;
  temp.y = 0.0;
  for (i=0; i < aisizes[id]; i++) {
    temp.x = temp.x + data[id][i].position.x;
    temp.y = temp.y + data[id][i].position.y;
  }
  // don't do this if there where no agents seen
  if (i > 0) {
    temp.x = temp.x / i;
    temp.y = temp.y / i;
  }
  temp.x = temp.x / CENTREOFMASSFRACT;
  temp.y = temp.y / CENTREOFMASSFRACT;
  accel.x = temp.x;
  accel.y = temp.y;

  //** repulsion rule
  temp.x = 0.0;
  temp.y = 0.0;
  for (i=0; i < aisizes[id]; i++) {
    // get around address space problems in opencl
    vector pos = data[id][i].position;
    if (magnitute2(&pos) < (REPULSIONDIST * REPULSIONDIST)) {
        temp.x = temp.x - pos.x;
        temp.y = temp.y - pos.y;
    }
  }
  temp.x = temp.x / REPULSIONFRACT;
  temp.y = temp.y / REPULSIONFRACT;
  accel.x = temp.x + accel.x;
  accel.y = temp.y + accel.y;

  //** mean velocity rule
  temp.x = 0.0;
  temp.y = 0.0;
  for (i=0; i < aisizes[id]; i++) {
    temp.x = temp.x + data[id][i].velocity.x;
    temp.y = temp.y + data[id][i].velocity.y;
  }
  // don't do this if there where no agents seen
  if (i > 0) {
    temp.x = temp.x / i;
    temp.y = temp.y / i;
  }
  temp.x = temp.x - velocity[id].x;
  temp.y = temp.y - velocity[id].y;
  temp.x = temp.x / MEANVELFRACT;
  temp.y = temp.y / MEANVELFRACT;
  accel.x = temp.x + accel.x;
  accel.y = temp.y + accel.y;

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
  temp = velocity[id];
  // use temp.x as a float instead of making another var
  temp.x = magnitute2 (&temp);
  if (temp.x > SPEEDLIMIT2) {
    temp.x = temp.x/SPEEDLIMIT2;
    velocity[id].x = velocity[id].x / temp.x;
    velocity[id].y = velocity[id].y / temp.x;
  }
}
