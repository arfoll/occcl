/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

__kernel void mandelbrot (__global char *data, __global float *job) 
{ 
  int table_char[] = { 32, 46, 44, 42, 126, 42, 94, 58, 59, 124, 38, 91, 36, 37, 64, 35 }; 
  const int idx = get_global_id(0); 
  if (idx < 100) { 
    float real = (((idx - 50) / (job[1] * 2.0)) - job[3]); 
    float imag = job[4]; 
    float iter_real = 0.0; 
    float iter_imag = 0.0; 
    int count = 0; 
    while ((((iter_real*iter_real)+(iter_imag*iter_imag)) < 32.0) && (count < 240)) { 
      float iter_real2 = iter_real; 
      float iter_imag2 = iter_imag; 
      float iter_r; 
      float iter_i; 
      iter_r = ((iter_real*iter_real2) - (iter_imag*iter_imag2)); 
      iter_i = ((iter_imag*iter_real2) + (iter_real*iter_imag2)); 
      iter_real = (real + iter_r); 
      iter_imag = (imag + iter_i); 
      count = count + 1; 
    } 
    int val = count % 16;
    data[idx*2] = (char) (val % 6);
    data[(idx*2)+1] = (char) table_char[val];
  } 
}
