/* Simple library meant to be called from occam that serves as proof
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "occoids_cl.h"

float magnitude2 (vector *ve)
{
  return (ve->x * ve->x) + (ve->y * ve->y);
}

float anglediff(float a, float b)
{
  float r = a - b;
  if (r < (-PI)) {
    r = r + (2.0 * PI);
  }
  else {
    r = r - (2.0 * PI);
  }
//  return fabs(r);
  return r;
}

__kernel void occoids (__global agentinfo (*data)[DIM1*DIM2], __global vector *velocity, __global int *rawsizes)
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

//////// filter.infos
  // dont really need these vars
  vector pos, vel;
  int aisize = 0; 
  vel = velocity[id];
  temp.x = atan2(vel.x, vel.y);

  // check this calculation is not worthless
  for (i=0; i < rawsizes[id]; i++) {
#if 0
     // works if filter.infos is run by PROC boid
     if (data[id][i].type == ATBOID) {
       data[id][aisize] = data[id][i];
       aisize++;
     }
#else
     pos = data[id][i].position;
     if (magnitude2(&pos) > (VISIONRADIUS2)) {
       //nothing
     }
     else if (data[id][i].type == ATCYLINDER) {
       //nothing
       //TODO: do cylinder stuff
     }
     else if ((magnitude2(&vel)) < EPSILON) {
       data[id][aisize] = data[id][i];
       aisize++;
     }
     else if ((anglediff(atan2(pos.x, pos.y), temp.x)) > VISIONMAXANGULARDIFF) {
       //nothing
     } 
     else {
       data[id][aisize] = data[id][i];
       aisize++;
     }
#endif
  }
/////// endof filter.infos

  //** centre of mass rule
  temp.x = 0.0;
  temp.y = 0.0;
  for (i=0; i < aisize; i++) {
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
  for (i=0; i < aisize; i++) {
    // get around address space problems in opencl
    vector pos = data[id][i].position;
    if (magnitude2(&pos) < (REPULSIONDIST * REPULSIONDIST)) {
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
  for (i=0; i < aisize; i++) {
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
  temp.x = magnitude2 (&temp);
  if (temp.x > SPEEDLIMIT2) {
    temp.x = temp.x/SPEEDLIMIT2;
    velocity[id].x = velocity[id].x / temp.x;
    velocity[id].y = velocity[id].y / temp.x;
  }
}
