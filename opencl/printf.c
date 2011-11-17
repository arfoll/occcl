/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "printf.h"

void printf_job (float *job, int length)
{
  int i = 0;
  for (i = 0; i < length; i++) {
    fprintf(stderr, "%f ", job[i]);
  }
  fprintf(stderr, "\n");
}

void _printfjob (int *w)
{ 
  printf_job ((float*) (w[0]), (int) (w[1]));
}

