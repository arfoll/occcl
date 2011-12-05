/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "sin.h"

const char sin_cl[] = "                             \
        __kernel void sin_cl  (global float *data)  \
        {                                           \
          int id = get_global_id(0);                \
          data[id] = sin(data[id]);                 \
        }                                           \
";

static int sin_init = 0;
static cl_context *context;
static cl_device_id *device;
static cl_program prog;
static cl_kernel k_sin;
static cl_command_queue *cq;

int clsin (cl_float *data) {
  cl_int error;
  cl_mem buffer;

#if ERROR_CHECK
  if (prog == NULL) {
    init_sin();
  }
#endif

  // Setup the input
  buffer = clCreateBuffer(*context, CL_MEM_COPY_HOST_PTR, sizeof(cl_float)*10240, data, &error);

  // Execute the kernel
  error = clSetKernelArg(k_sin, 0, sizeof(buffer), &buffer);
  size_t global_dimensions[] = {10240,0,0};
  error = clEnqueueNDRangeKernel(*cq, k_sin, 1, NULL, global_dimensions, NULL, 0, NULL, NULL);

  // Read back the results
  error = clEnqueueReadBuffer(*cq, buffer, CL_TRUE, 0, sizeof(cl_float)*10240, data, 0, NULL, NULL);

  // Clean up
  clReleaseMemObject(buffer);

  // return the ciphertext 
  return error;
}

int init_sin ()
{
  cl_int error;

  if (sin_init)
    return 1;

  context = get_cl_context();
  device = get_cl_device();

  const char *src=sin_cl;
  size_t srcsize=strlen(sin_cl);
  const char *srcptr[]={src};

  // build CL program
  error = buildcl (srcptr, &srcsize, &prog, "");
  // create kernel
  k_sin = clCreateKernel(prog, "sin_cl", &error);
  // get the shared CQ
  cq = get_command_queue();

  if (!error)
    sin_init = 1;

  return error;
}
