/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "printf.h"

void printf_job (cl_float *job, cl_int length)
{
  int i = 0;
  for (i = 0; i < length; i++) {
    fprintf(stderr, "%f ", job[i]);
  }
  fprintf(stderr, "\n");
}

void _printfjob (int *w)
{ 
  printf_job ((cl_float*) (w[0]), (cl_int) (w[1]));
}

// Occam has a bug when casting the worspace pointer to a single float/double 
// so we require a real array (can be any length, will be printed in its entirity)
void _printfreal (int *w)
{
  int i;
  cl_float *var = (cl_float*)(w[0]); 
  int size = w[1];
  for (i=0; i < size; i++)
    fprintf(stderr, "%f\n", *var++);
}

void _printfreal64 (int *w)
{
  int i;
  double *var = (double*)(w[0]);
  int size = w[1];
  for (i=0; i < size; i++)
    fprintf(stderr, "%f\n", *var++);
}

void _printfint (int *w)
{
  fprintf(stderr, "%d\n", (cl_int) (w[0]));
}
