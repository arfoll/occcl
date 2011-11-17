/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "mandelbrot.h"

cl_char table[] = " .,*~*^:;|&[$%@#";
#if 1
cl_int table_int[] = { 32, 46, 44, 42, 126, 42, 94, 58, 59, 124, 38, 91, 36, 37, 64, 35 };
#else
cl_int table_int[] = {  0,  1,  2,  3,   4,  5,  6,  7,  8,   9, 10, 11, 12, 13, 14, 15 };
#endif

#if 1 
const char mandelbrot_cl[] =
"#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable \n\
__kernel void mandelbrot (__global char *data, __global float *job) \n\
{ \n\
  int table_char[] = { 32, 46, 44, 42, 126, 42, 94, 58, 59, 124, 38, 91, 36, 37, 64, 35 }; \n\
  const uint idx = get_global_id(0); \n\
  if (idx < 100) { \n\
    float real = ((idx - ((100)/2)) / (job[1] * 2.0)) - job[3]; \n\
    float imag = job[4]; \n\
    float iter_real = 0.0; \n\
    float iter_imag = 0.0; \n\
    int count = 0; \n\
    while ((((iter_real*iter_real)+(iter_imag*iter_imag)) < 32.0) && (count < 240)) { \n\
      float iter_real2 = iter_real; \n\
      float iter_imag2 = iter_imag; \n\
      float iter_r; \n\
      float iter_i; \n\
      iter_r = (iter_real*iter_real2) - (iter_imag*iter_imag2); \n\
      iter_i = (iter_imag*iter_real2) + (iter_real*iter_imag2); \n\
      iter_real = real + iter_r; \n\
      iter_imag = imag + iter_i; \n\
      count = count + 1; \n\
    } \n\
    int val = count % 16; \n\
    data[idx*2] = (char) (val % 6); \n\
    data[(idx*2)+1] = (char) table_char[val]; \n\
  } \n\
}";
#else
    data[idx*2] = (char) (val % 6); \n\
    data[(idx*2)+1] = (char) table_char[val]; \n\
const char mandelbrot_cl[] = " \
__kernel void mandelbrot (__global char *data, __global float *job)\
{ \
  int table_int[] = { 32, 46, 44, 42, 126, 42, 94, 58, 59, 124, 38, 91, 36, 37, 64, 35 }; \n\
  int idx = get_global_id(0); \
  if (idx < 100) { \
    char x = table_int[idx % 16]; \
    data[idx*2] = x; \
    data[(idx*2)+1] = x;\
  } \
}";
#endif

cl_fract COMPLEX64ABSSQ (float complex c)
{
  cl_fract real = __real__ c;
  cl_fract imag = __imag__ c;
  return (real*real) + (imag*imag);
}

#if 0
int calc (float complex c)
{
  float complex iter;
  __real__ iter = 0.0;
  __imag__ iter = 0.0;
  int count = 0;
  while ((((COMPLEX64ABSSQ(iter))) < 32.0) && (count < 240)) {
    iter = (iter * iter) + c;
    count++;
  }
  return count;
}
#endif

void mandelbrot_c (cl_char *data, cl_fract *job, cl_int width)
{
  int i = 0;
  cl_fract y = job[0]/job[1] - job[2];
  fprintf (stderr, "native job0 = %f, job1 = %f, job2, %f, job3 %f, y = %f\n", job[0], job[1], job[2], job[3], y);
  for (i = 0; i < width; i++) {
    cl_float real = ((i - ((width)/2)) / (job[1] * 2.0)) - job[3];
    cl_float imag = y;
    //int val = calc (c) % 16;
    cl_float iter_real = 0.0;
    cl_float iter_imag = 0.0;
    int count = 0;
    while ((((iter_real*iter_real)+(iter_imag*iter_imag)) < 32.0) && (count < 240)) {
      cl_float iter_real2 = iter_real;
      cl_float iter_imag2 = iter_imag;
      cl_float iter_r;
      cl_float iter_i;
      iter_r = (iter_real*iter_real2) - (iter_imag*iter_imag2);
      iter_i = (iter_imag*iter_real2) + (iter_real*iter_imag2);
      iter_real = real + iter_r;
      iter_imag = imag + iter_i;
      count++;
    }
    int val = count;
    //fprintf (stdout, "count: %d\n", val);
    data[i] = (cl_char) val;
    //data[i*2] = (char) (val % 6);
    //data[(i*2)+1] = table_int[val];
  }
}

static int mandelbrot_init = 0;
static cl_context* context;
static cl_device_id* device;
static cl_program prog;
static cl_kernel k_mandelbrot;

void _mandelbrot (int *w)
{ 
#if 0
  mandelbrot_c ((cl_char*) (w[0]), (cl_fract*) (w[2]), (cl_int) (w[4]));
#else
  cl_fract *job = (cl_fract*) (w[2]);
  cl_fract job_y[5];
  job_y[0] = job[0];
  job_y[1] = job[1];
  job_y[2] = job[2];
  job_y[3] = job[3];
  job_y[4] = job[0]/job[1] - job[2];
  mandelbrot ((cl_char*) (w[0]), (cl_fract*) (&job_y), (cl_int) (w[4]));
#endif
}

void _initmandelbrot (int *w)
{
  init_mandelbrot();
}

int mandelbrot (cl_char *data, cl_fract *job, cl_int width) {
  
  cl_int error;

#if ERROR_CHECK
  if (prog == NULL) {
    init_mandelbrot();
  }
#endif

#if 0
  fprintf (stderr, "opencl job0 = %f, job1 = %f, job2, %f, job3 %f, y = %f\n", 
           job[0], job[1], job[2], job[3], job[4]);
#endif

  // create command queue
  cl_command_queue cq = clCreateCommandQueue(*context, *device, 0, &error);

  // Allocate memory for the kernel to work with
  cl_mem mem1, mem2;
  mem1 = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizeof(cl_char)*(width*2), 0, &error);
  mem2 = clCreateBuffer(*context, CL_MEM_COPY_HOST_PTR, sizeof(cl_fract)*5, job, &error);
  
  // get a handle and map parameters for the kernel
  error = clSetKernelArg(k_mandelbrot, 0, sizeof(cl_mem), &mem1);
  error = clSetKernelArg(k_mandelbrot, 1, sizeof(cl_mem), &mem2);

  // Perform the operation (width is 100 in this example)
  size_t worksize = width;
  error = clEnqueueNDRangeKernel(cq, k_mandelbrot, 1, NULL, &worksize, 0, 0, 0, 0);
  // Read the result back into data
  error = clEnqueueReadBuffer(cq, mem1, CL_TRUE, 0, (size_t) (width*2), data, 0, 0, 0);

  // cleanup and wait for release of cq
  clReleaseMemObject(mem1);
  clReleaseMemObject(mem2);

  error = clFinish(cq);
  
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
