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
  __local float r;
  r = a - b;
  if (r < (-PI)) {
    r = r + (2.0 * PI);
  }
  else {
    r = r - (2.0 * PI);
  }
  //return fabs(r);
  return r;
}

__kernel void occoids (__global agentinfo (*data)[DIM1*DIM2], __global vector *velocity, __constant int *rawsizes)
{
  int4 id;
  // our 2D ids are mapped to x&y
  id.x = get_global_id(0);
  id.y = get_global_id(1);
  // combined to give a single id so we can troll through our *data array
  id.z = id.x + (id.y * DIM1);

  // Vars we work with. To reduce memory usage we reuse variables violently
  int i;
  vector temp;
  vector accel;

//////// filter.infos
  // dont really need these vars
  vector pos, vel;
  int aisize = 0; 
  vel = velocity[id.z];
  temp.x = atan2(vel.x, vel.y);

  // check this calculation is not worthless
  for (i=0; i < rawsizes[id.z]; i++) {
#if 0     
     // works if filter.infos is run by PROC boid
     if (data[id.z][i].type == ATBOID) {
       data[id.z][aisize] = data[id.z][i];
       aisize++;
     }
#else
     pos = data[id.z][i].position;
     if (magnitude2(&pos) > (VISIONRADIUS2)) {
       //nothing
     }
     else if (data[id.z][i].type == ATCYLINDER) {
       //nothing
       //TODO: do cylinder stuff
     }
     else if ((magnitude2(&vel)) < EPSILON) {
       data[id.z][aisize] = data[id.z][i];
       aisize++;
     }
     else if ((anglediff(atan2(pos.x, pos.y), temp.x)) > VISIONMAXANGULARDIFF) {
       //nothing
     } 
     else {
       data[id.z][aisize] = data[id.z][i];
       aisize++;
     }
#endif
  }
/////// endof filter.infos

  //** centre of mass rule
  temp = (float2) (0.0f, 0.0f);
  for (i=0; i < aisize; i++) {
    temp.x = temp.x + data[id.z][i].position.x;
    temp.y = temp.y + data[id.z][i].position.y;
  }
  // don't do this if there where no agents seen
  if (i > 0) {
    accel = temp / i;
    accel = accel / CENTREOFMASSFRACT;
  }

  //** repulsion rule
  temp = (float2) (0.0f, 0.0f);
  for (i=0; i < aisize; i++) {
    // get around address space problems in opencl
    vector pos = data[id.z][i].position;
    if (magnitude2(&pos) < (REPULSIONDIST * REPULSIONDIST)) {
        temp.x = temp.x - pos.x;
        temp.y = temp.y - pos.y;
    }
  }
  temp = temp / REPULSIONFRACT;
  accel.x = temp.x + accel.x;
  accel.y = temp.y + accel.y;

  //** mean velocity rule
  temp = (float2) (0.0f, 0.0f);
  for (i=0; i < aisize; i++) {
    temp.x = temp.x + data[id.z][i].velocity.x;
    temp.y = temp.y + data[id.z][i].velocity.y;
  }
  // don't do this if there where no agents seen
  if (i > 0) {
    temp = temp / i;
    temp.x = temp.x - velocity[id.z].x;
    temp.y = temp.y - velocity[id.z].y;
    temp = temp / MEANVELFRACT;
    accel.x = temp.x + accel.x;
    accel.y = temp.y + accel.y;
  }

  //TODO: add obstacle rule

  //** accelerate
  accel = accel / SMOOTHACCEL;
  velocity[id.z].x = velocity[id.z].x + (accel.x);
  velocity[id.z].y = velocity[id.z].y + (accel.y);

#if 1
  if (fabs(velocity[id.z].x) < 0.00000) {
    velocity[id.z].x = 0.0;
  }
  if (fabs(velocity[id.z].y) < 0.00000) {
    velocity[id.z].y = 0.0;
  }
#endif

  // get around address space problems in opencl
  temp = velocity[id.z];
  // use temp.x as a float instead of making another var
  temp.x = magnitude2 (&temp);
  if (temp.x > SPEEDLIMIT2) {
    temp.x = temp.x/SPEEDLIMIT2;
    velocity[id.z].x = velocity[id.z].x / temp.x;
    velocity[id.z].y = velocity[id.z].y / temp.x;
  }
}
