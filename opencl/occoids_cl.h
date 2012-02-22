/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#ifndef OCCOIDS_CL_H
#define OCCOIDS_CL_H

#define ATBOID 1
#define ATCYLINDER 2
#define CENTREOFMASSFRACT 45.0
#define REPULSIONDIST 0.06
#define REPULSIONFRACT 4.0
#define MEANVELFRACT 8.0
#define OBSTACLEFRACT 8.0
#define SOFTTHRESHOLD 0.05
#define SMOOTHACCEL 5.0
#define SPEEDLIMIT 0.03
#define SPEEDLIMIT2 (SPEEDLIMIT * SPEEDLIMIT) //0.0009
#define VISIONRADIUS 0.25
#define VISIONANGLE 200.0
#define VISIONRADIUS2 VISIONRADIUS*VISIONRADIUS
#define EPSILON 1.0E-8
// from nvidia cuda math lib. Is there an inbuilt for opencl?
#define PI 3.14159274101257f
#define VISIONMAXANGULARDIFF ((VISIONANGLE / 2.0) * PI) / 180.0

typedef struct {
  float x;
  float y;
} vector;

typedef struct {
  int localid;
  int type;
  vector position;
  vector velocity;
  float radius;
  int colour;
} agentinfo;

#endif
