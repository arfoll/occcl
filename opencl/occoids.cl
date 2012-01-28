/* Simple library meant to be called from occam that serves as proof
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void occoids (__global char *data, __global double *job)
{
  char table_char[] = " .,*~*^:;|&[$%@#";
  //int table_char[] = { 32, 46, 44, 42, 126, 42, 94, 58, 59, 124, 38, 91, 36, 37, 64, 35 };
  const int idx = get_global_id(0);
  double real = (((idx - 50) / (job[1] * 2.0)) - job[3]);
  double imag = job[4];
  double iter_real = 0.0;
  double iter_imag = 0.0;
  int count = 0;
  while ((((iter_real*iter_real)+(iter_imag*iter_imag)) < 32.0) && (count < 240)) {
    double iter_r;
    double iter_i;
    iter_r = ((iter_real*iter_real) - (iter_imag*iter_imag));
    iter_i = ((iter_imag*iter_real) + (iter_real*iter_imag));
    iter_real = (real + iter_r);
    iter_imag = (imag + iter_i);
    count++;
  }
  int val = count % 16;
  data[idx*2] = (char) (val % 6);
  data[(idx*2)+1] = table_char[val];
}
