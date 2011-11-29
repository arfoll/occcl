/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

__kernel void mandelbrot (__global char *data, __global float *job) 
{ 
  char table_char[] = " .,*~*^:;|&[$%@#";
  const int idx = get_global_id(0); 
  float real = ((convert_float4_rtp(idx - 50)) / (job[1] * 2.0)) - job[3]; 
  float imag = job[4]; 
  float iter_real = 0.0; 
  float iter_imag = 0.0; 
  int count = 0; 
  while ((((iter_real*iter_real)+(iter_imag*iter_imag)) < 32.0) && (count < 240)) { 
    float iter_r = 0.0; 
    float iter_i = 0.0; 
    iter_r = ((iter_real*iter_real) - (iter_imag*iter_imag)); 
    iter_i = ((iter_imag*iter_real) + (iter_real*iter_imag)); 
    iter_real = (real + iter_r); 
    iter_imag = (imag + iter_i); 
    count++; 
  } 
  int val = count % 16;
  data[idx*2] = convert_char4_sat(val % 6);
  data[(idx*2)+1] = table_char[val];
}
