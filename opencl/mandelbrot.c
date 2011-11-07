/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "mandelbrot.h"

#if 0
const char mandelbrot_cl[] = "                  \
__kernel void mandelbrot                        \
    (   __global    const   char    *output,    \
        __global    const   double  *job,       \
        __global    const   int     width       \
    )                                           \
{                                               \
    const uint index = get_global_id(0);        \
    if (index < width) {                        \
      \
}             \
";
#endif

#if 0
const char mandelbrot_cl[] = "                    \
__kernel void mandelbrot_calc                     \
   (__global    const   complex double  a,        \
    __global    const   int       count)          \
{                                                 \
    const uint index = get_global_id(0);          \
    complex double b = 0.0;                       \
    while (((b.real*b.real+b.img*b.img) < 32.0)   \
           && (count < 240)) {                    \
      b = b*b + a;                                \
      count++;                                    \
    }                                             \
}";
#endif

const char mandelbrot_cl[] =
"__kernel void mandelbrot (__global char *data, __global float *job, __global const int *width)\
{\
  const uint index = get_global_id(0); \
  if (index < width) { \
     data[index] = 5 + index; \
  } \
}";

static int mandelbrot_init = 0;
static cl_context* context;
static cl_device_id* device;
static cl_program prog;
static cl_kernel k_mandelbrot;

void _mandelbrot (int *w)
{ 
  mandelbrot ((cl_char*) (w[0]), (cl_float*) (w[1]), (cl_int*) (w[2]));
}

int mandelbrot (cl_char *data, cl_float *job, cl_int *width) {
  
  cl_int error;

#if ERROR_CHECK
  if (prog == NULL) {
    init_mandelbrot();
  }
#endif

  // create command queue
  cl_command_queue cq = clCreateCommandQueue(*context, *device, 0, &error);

  // Allocate memory for the kernel to work with
  cl_mem mem1, mem2, mem3;
  mem1 = clCreateBuffer(*context, CL_MEM_COPY_HOST_PTR, sizeof(cl_char)*(*width), data, &error);
  mem2 = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(cl_float)*4, job, &error);
  mem3 = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(cl_int), width, &error);
  
  // get a handle and map parameters for the kernel
  error = clSetKernelArg(k_mandelbrot, 0, sizeof(mem1), &mem1);
  error = clSetKernelArg(k_mandelbrot, 1, sizeof(mem2), &mem2);
  error = clSetKernelArg(k_mandelbrot, 2, sizeof(mem3), &mem3);

  // Perform the operation (width is 100 in this example)
  size_t global_dimensions[] = {100,0,0};
  error = clEnqueueNDRangeKernel(cq, k_mandelbrot, 1, NULL, global_dimensions, NULL, 0, NULL, NULL);
  // Read the result back into ciphertext 
  error = clEnqueueReadBuffer(cq, mem1, CL_TRUE, 0, sizeof(cl_char)*(*width), data, 0, NULL, NULL);

  // cleanup and wait for release of cq
  clReleaseMemObject(mem1);
  clReleaseMemObject(mem2);
  clReleaseMemObject(mem3);
  error=clFinish(cq);
  
  // return the ciphertext 
  return error;
}

int init_mandelbrot ()
{
  cl_int error;

  if (mandelbrot_init) {
    // if called again should we do nothing or restart init from scratch?
    return 1;
  }

  //error = initialisecl();
  context = get_cl_context();
  device = get_cl_device();

  const char *src=mandelbrot_cl;
  size_t srcsize=strlen(mandelbrot_cl);
  const char *srcptr[]={src};

  // build CL program
  error = buildcl (srcptr, &srcsize, &prog);
  // create kernel
  k_mandelbrot = clCreateKernel(prog, "mandelbrot", &error);

  // we are initialised
  // TODO: use error val to calculate this
  mandelbrot_init = 1;
  return error;
}
