/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "mandelbrot_cl.h"

#if __OPENCL_VERSION__ < 110
    #pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
#endif

#ifndef cl_khr_byte_addressable_store
    #error "This kernel requires byte-addressable store"
#endif

__kernel void mandelbrot (__global char (*data)[50][200], __global floatcl *job)
{
  char table_char[] = " .,*~*^:;|&[$%@#";
  const int idy = get_global_id(2); 
  const int idx = get_global_id(1);
  const int idz = get_global_id(0);
  int idzi = idz*5;

  // this should not be done in the CL kernel
  job[idzi] = (floatcl) idy - 25;
  floatcl y = job[idzi]/job[idzi+1] - job[idzi+2];

  floatcl real = (((idx - 50) / (job[idzi+31] * 2.0)) - job[idzi+3]); 
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
  int val = count % 16;

  data[idz][idy][idx*2] = (char) (val % 6);
  data[idz][idy][(idx*2)+1] = table_char[val];
}

#if 0
__kernel void initialiseJobs (__global floatcl *jobs)
{
  floatcl zoom = 16.0;
  floatcl xdrift = 0.0;
  floatcl ydrift = 0.0;
  floatcl xtarget = 1.16000014859;
  floatcl ytarget = -0.27140215303;

  // iterates 850 times over all jobs
  const uint idx = get_global_id(0);
  // iterates LENGTH times for every job
  const uint idy = get_global_id(1);

  uint index = 5*idx;
  jobs[(index)+0] = idy - 25;
  jobs[index+1] = ZOOM + ((zoom / 32.0) * idx);
  jobs[index+2] = ydrift + ((diffx / 16.0) * idx);
  jobs[index+3] = xdrift + ((diffy / 16.0) * idx);
  jobs[index+4] = jobs[index]/jobs[index+1] - jobs[index+2];
}
#endif
