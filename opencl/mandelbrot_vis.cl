/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "mandelbrot_cl.h"

__kernel void mandelbrot_vis(__global int (*data)[IMAGEHEIGHTVIS][IMAGEWIDTHVIS], __constant floatcl *job)
{
  int8 id;
  id.y = get_global_id(0); 
  id.x = get_global_id(1);
  id.z = get_global_id(2);
  id.w = id.z*3;

  floatcl y = (floatcl) id.y - (IMAGEHEIGHTVIS/2);
  y = y/job[id.w] - job[id.w+1];

  floatcl4 var;
  var.x = (((id.x - IMAGEHEIGHTVIS) / (job[id.w] * 2.0f)) - job[id.w+2]);
  var.y = y;
  floatcl2 iter = (floatcl2) (0.0f, 0.0f);
  int count = 0; 
  while ((count < 240) && (((iter.x*iter.x)+(iter.y*iter.y)) < 32.0f)) {
    var.z = ((iter.x*iter.x) - (iter.y*iter.y)); 
    var.w = ((iter.y*iter.x) + (iter.x*iter.y)); 
    iter.x = (var.x + var.z); 
    iter.y = (var.y + var.w); 
    count++; 
  }
  count = count & 15;
  data[id.z][id.y][id.x] = -268435456 >> (count*2);
}

