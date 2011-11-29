/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "modulo.h"

const char modulo_cl[] = " \
__kernel void modulo (__global int *num, __global float *mod, __global float *amount) \n\
{ \n\
  (*mod) = ((*num)-50) / (*amount * 2.0); \n\
  (*mod) = ((*mod) * (*mod)) * (*mod); \n\
}";

static int modulo_init = 0;
static cl_context* context;
static cl_device_id* device;
static cl_program prog;
static cl_kernel k_modulo;

int modulo (cl_int *num, cl_float *mod, cl_float *amount)
{
  cl_int error;
  size_t worksize = 1;
  cl_command_queue cq = clCreateCommandQueue(*context, *device, 0, &error);
  cl_mem mem1, mem2, mem3;
  mem1=clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(cl_int), NULL, &error);
  mem2=clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizeof(cl_float), NULL, &error);
  mem3=clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(cl_float), NULL, &error);

  clSetKernelArg(k_modulo, 0, sizeof(mem1), &mem1);
  clSetKernelArg(k_modulo, 1, sizeof(mem2), &mem2);
  clSetKernelArg(k_modulo, 2, sizeof(mem3), &mem3);

  error=clEnqueueWriteBuffer(cq, mem1, CL_FALSE, 0, sizeof(cl_int), num, 0, NULL, NULL);
  error=clEnqueueWriteBuffer(cq, mem3, CL_FALSE, 0, sizeof(cl_float), amount, 0, NULL, NULL);

  error=clEnqueueNDRangeKernel(cq, k_modulo, 1, NULL, &worksize, &worksize, 0, NULL, NULL);

  error=clEnqueueReadBuffer(cq, mem2, CL_FALSE, 0,  sizeof(cl_float), mod, 0, NULL, NULL);
  error=clFinish(cq);

  return error;
}

int init_modulo() 
{
  cl_int error;

  if (modulo_init)
    return 1;

  context = get_cl_context();
  device = get_cl_device();

  const char *src=modulo_cl;
  size_t srcsize=strlen(modulo_cl);
  const char *srcptr[]={src};

  // build CL program
  error = buildcl (srcptr, &srcsize, &prog, "");
  // create kernel
  k_modulo = clCreateKernel(prog, "modulo", &error);

  if (!error)
    modulo_init = 1;

  return error;
}
