/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "rot13.h"

const char rot13_cl[] = 
"#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable \n\
  __kernel void rot13 (__global const char* in, __global char* out ) \n\
{ \n\
    const uint index = get_global_id(0); \n\
    char c=in[index]; \n\
    if (c<'A' || c>'z' || (c>'Z' && c<'a')) { \n\
        out[index] = in[index]; \n\
    } else { \n\
      if (c>'m' || (c>'M' && c<'a')) { \n\
        out[index] = in[index]-13; \n\
      } else { \n\
        out[index] = in[index]+13; \n\
      } \n\
    } \n\
}";

static int rot13_init = 0;
static cl_context* context;
static cl_device_id* device;
static cl_program prog;
static cl_kernel k_rot13;

void _rot13 (int *w)
{ 
  // w[1] and w[3] and strlen
  rot13 ((char *) (w[0]), (char *) (w[2]));
}

void _initrot13 (int *w)
{
  init_rot13();
}

int rot13 (char* plaintext, char* ciphertext) {
  size_t worksize=strlen(plaintext);
  
  cl_int error;

#if ERROR_CHECK
  if (prog == NULL) {
    init_rot13();
  }
#endif

  // create command queue
  cl_command_queue cq = clCreateCommandQueue(*context, *device, 0, &error);

  // Allocate memory for the kernel to work with
  cl_mem mem1, mem2;
  mem1=clCreateBuffer(*context, CL_MEM_READ_ONLY, worksize, NULL, &error);
  mem2=clCreateBuffer(*context, CL_MEM_WRITE_ONLY, worksize, NULL, &error);
  
  // get a handle and map parameters for the kernel
  clSetKernelArg(k_rot13, 0, sizeof(mem1), &mem1);
  clSetKernelArg(k_rot13, 1, sizeof(mem2), &mem2);

  // Target buffer just so we show we got the data from OpenCL
  ciphertext[0]='?';
  ciphertext[worksize]=0;

  // Send input data to OpenCL (async, don't alter the buffer!)
  error=clEnqueueWriteBuffer(cq, mem1, CL_FALSE, 0, worksize, plaintext, 0, NULL, NULL);
  // Perform the operation
  error=clEnqueueNDRangeKernel(cq, k_rot13, 1, NULL, &worksize, &worksize, 0, NULL, NULL);
  // Read the result back into ciphertext 
  error=clEnqueueReadBuffer(cq, mem2, CL_FALSE, 0, worksize, ciphertext, 0, NULL, NULL);
  // Await completion of all the above
  error=clFinish(cq);
  
  // return the ciphertext 
  return error;
}

int init_rot13 ()
{
  cl_int error;

  if (rot13_init)
    return 1;

  context = get_cl_context();
  device = get_cl_device();

  const char *src=rot13_cl;
  size_t srcsize=strlen(rot13_cl);
  const char *srcptr[]={src};

  // build CL program
  error = buildcl (srcptr, &srcsize, &prog, "");
  // create kernel
  k_rot13 = clCreateKernel(prog, "rot13", &error);

  // we are initialised
  if (!error)
    rot13_init = 1;

  return error;
}
