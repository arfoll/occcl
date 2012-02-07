/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "mandelbrot_cl.h"

__kernel void mandelbrot_vis(__global int (*data)[IMAGEHEIGHTVIS][IMAGEWIDTHVIS], __global floatcl *job)
{
  const int idy = get_global_id(0); 
  const int idx = get_global_id(1);
  const int idz = get_global_id(2);
  int idzi = idz*5;

  job[idzi] = (floatcl) idy - (IMAGEHEIGHTVIS/2);
  floatcl y = job[idzi]/job[idzi+1] - job[idzi+2];

  floatcl real = (((idx - IMAGEHEIGHTVIS) / (job[idzi+1] * 2.0)) - job[idzi+3]); 
  floatcl imag = y;
  floatcl iter_real = 0.0; 
  floatcl iter_imag = 0.0; 
  int count = 0; 
  while ((((iter_real*iter_real)+(iter_imag*iter_imag)) < 32.0) && (count < 240)) {
    floatcl iter_r; 
    floatcl iter_i; 
    iter_r = ((iter_real*iter_real) - (iter_imag*iter_imag)); 
    iter_i = ((iter_imag*iter_real) + (iter_real*iter_imag)); 
    iter_real = (real + iter_r); 
    iter_imag = (imag + iter_i); 
    count++; 
  }
  int val = count & 15;
  data[idz][idy][idx] = -268435456 >> (val*2);
}
