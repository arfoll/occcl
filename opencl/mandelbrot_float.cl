/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#ifndef cl_khr_byte_addressable_store
    #error This kernel requires byte-addressable store
#endif

__kernel void mandelbrot (__global char (*data)[200], __global float *job)
{
  char table_char[] = " .,*~*^:;|&[$%@#";
  const int idy = get_global_id(0); 
  const int idx = get_global_id(1);

  // this should not be done in the CL kernel
  job[0] = (float) idy - 25;
  float y = job[0]/job[1] - job[2];

  float real = (((idx - 50) / (job[1] * 2.0)) - job[3]); 
  float imag = y;
  float iter_real = 0.0; 
  float iter_imag = 0.0; 
  int count = 0; 
  while ((((iter_real*iter_real)+(iter_imag*iter_imag)) < 32.0) && (count < 240)) {
    float iter_r; 
    float iter_i; 
    iter_r = ((iter_real*iter_real) - (iter_imag*iter_imag)); 
    iter_i = ((iter_imag*iter_real) + (iter_real*iter_imag)); 
    iter_real = (real + iter_r); 
    iter_imag = (imag + iter_i); 
    count++; 
  }
  int val = count % 16;

  data[idy][idx*2] = (char) (val % 6);
  data[idy][(idx*2)+1] = table_char[val];
}
