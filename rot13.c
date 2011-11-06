/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - bl73@kent.ac.uk
 */

#include <stdio.h>
#include <string.h>

#include "initial.h"

const char rot13_cl[] = "       \
__kernel void rot13         \
    (   __global    const   char*    in     \
    ,   __global            char*    out    \
    )             \
{             \
    const uint index = get_global_id(0);    \
              \
    char c=in[index];         \
    if (c<'A' || c>'z' || (c>'Z' && c<'a')) {   \
        out[index] = in[index];       \
    } else {            \
        if (c>'m' || (c>'M' && c<'a')) {    \
      out[index] = in[index]-13;      \
  } else {          \
      out[index] = in[index]+13;      \
  }           \
    }             \
}             \
";

int rot13_init = 0;

char* rot13 (char* plaintext) {
  size_t srcsize, worksize=strlen(plaintext);
  
  cl_int error;
  cl_device_id device;
  cl_context context;
  cl_program prog;

  // CL initialisation
  error = initialisecl(&context, &device);
  errorMessageCL(error);
  // create command queue
  cl_command_queue cq = clCreateCommandQueue(context, device, 0, &error);

  const char *src=rot13_cl;
  srcsize = strlen(rot13_cl);
  const char *srcptr[]={src};
  // build CL program
  buildcl (srcptr, &srcsize, &context, &prog);

  // Allocate memory for the kernel to work with
  cl_mem mem1, mem2;
  mem1=clCreateBuffer(context, CL_MEM_READ_ONLY, worksize, NULL, &error);
  mem2=clCreateBuffer(context, CL_MEM_WRITE_ONLY, worksize, NULL, &error);
  
  // get a handle and map parameters for the kernel
  cl_kernel k_rot13=clCreateKernel(prog, "rot13", &error);
  clSetKernelArg(k_rot13, 0, sizeof(mem1), &mem1);
  clSetKernelArg(k_rot13, 1, sizeof(mem2), &mem2);

  // Target buffer just so we show we got the data from OpenCL
  char ciphertext[sizeof plaintext];
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
  return(ciphertext);
}

int init_rot13 ()
{
  if (rot13_init) {
    // if called again should we do nothing or restart init from scratch?
    return 1;
  }
}
