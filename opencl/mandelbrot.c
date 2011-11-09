/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "mandelbrot.h"

cl_char table[] = " .,\n~*^:;|&[$%@#";

#if 1 
const char mandelbrot_cl[] =
"__kernel void mandelbrot (__global char *data, __global float *job, __global int *width, \
                           __global float *y, __global char *table) \
{ \
  const uint idx = get_global_id(0); \
  if (idx < width) { \
    float i = *y; \
    float r = ((idx - ((*width)/2)) / (job[1] * 2.0)) - job[3]; \
    int val; \
    int count = 0; \
    while ((((r*r)+(i*i)) < 32.0) && (count < 240)) { \
      r = (r*i) - (r*i); \
      i = (r*i) + (r*i); \
      count++; \
    } \
    count = count % 16; \
    data[idx*2] = (char) (count % 6); \
    data[(idx*2)+1] = table[count]; \
  } \
}";
#else
const char mandelbrot_cl[] = " \
__kernel void mandelbrot (__global char *data, __global float *job, __global int *width, \
                          __global float *y, __global char *table)\
{ \
  const uint idx = get_global_id(0); \
  if (idx < width) { \
    float r = ((idx - ((*width)/2)) / (job[1] * 2.0)) - job[3]; \
    data[idx] = table[idx%16]; \
  } \
}";
#endif

cl_fract COMPLEX64ABSSQ (double complex c)
{
  cl_fract real = __real__ c;
  cl_fract imag = __imag__ c;
  return (real*real) + (imag*imag);
}

int calc (double complex c)
{
  double complex iter;
  __real__ iter = 0.0;
  __imag__ iter = 0.0;
  int count = 0;
  while ((((COMPLEX64ABSSQ(iter))) < 32.0) && (count < 240)) {
    iter = (iter * iter) + c;
    count++;
  }
  return count;
}

void mandelbrot_c (cl_char *data, cl_fract *job, cl_int width)
{
  int i = 0;
  cl_fract y = job[0]/job[1] - job[2];
  for (i = 0; i < width; i++) {
    cl_fract x = ((i - ((width)/2)) / (job[1] * 2.0)) - job[3];
    double complex c;
     __real__ c = x;
     __imag__ c = y;
    int val = calc (c) % 16;
    //fprintf (stdout, "count: %d\n", val);
    data[i*2] = (char) (val % 6);
    data[(i*2)+1] = table[val];
  }
}

static int mandelbrot_init = 0;
static cl_context* context;
static cl_device_id* device;
static cl_program prog;
static cl_kernel k_mandelbrot;

void _mandelbrot (int *w)
{ 
  //mandelbrot_c ((cl_char*) (w[0]), (cl_fract*) (w[2]), (cl_int) (w[4]));
  cl_fract *job = (cl_fract*) (w[2]);
  cl_fract y = job[0]/job[1] - job[2];
  mandelbrot ((cl_char*) (w[0]), job, (cl_int) (w[4]), &y);
}

void _initmandelbrot (int *w)
{
  init_mandelbrot();
}

int mandelbrot (cl_char *data, cl_fract *job, cl_int width, cl_fract *y) {
  
  cl_int error;

#if ERROR_CHECK
  if (prog == NULL) {
    init_mandelbrot();
  }
#endif

  // create command queue
  cl_command_queue cq = clCreateCommandQueue(*context, *device, 0, &error);

  // Allocate memory for the kernel to work with
  cl_mem mem1, mem2, mem3, mem4, mem5;
  mem1 = clCreateBuffer(*context, CL_MEM_COPY_HOST_PTR, sizeof(cl_char)*(width*2), data, &error);
  mem2 = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(cl_fract)*4, job, &error);
  mem3 = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(cl_int), &width, &error);
  mem4 = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(cl_fract), &y, &error);
  mem5 = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(cl_char)*17, &table[0], &error);
  
  // get a handle and map parameters for the kernel
  error = clSetKernelArg(k_mandelbrot, 0, sizeof(mem1), &mem1);
  error = clSetKernelArg(k_mandelbrot, 1, sizeof(mem2), &mem2);
  error = clSetKernelArg(k_mandelbrot, 2, sizeof(mem3), &mem3);
  error = clSetKernelArg(k_mandelbrot, 3, sizeof(mem4), &mem4);
  error = clSetKernelArg(k_mandelbrot, 4, sizeof(mem5), &mem5);

  // Perform the operation (width is 100 in this example)
  size_t global_dimensions[] = {width*2,0,0};
  error = clEnqueueNDRangeKernel(cq, k_mandelbrot, 1, NULL, global_dimensions, NULL, 0, NULL, NULL);
  // Read the result back into ciphertext 
  error = clEnqueueReadBuffer(cq, mem1, CL_TRUE, 0, sizeof(cl_char)*(width), data, 0, NULL, NULL);

  // cleanup and wait for release of cq
  clReleaseMemObject(mem1);
  clReleaseMemObject(mem2);
  clReleaseMemObject(mem3);
  clReleaseMemObject(mem4);
  clReleaseMemObject(mem5);
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
