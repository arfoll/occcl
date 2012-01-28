/* Simple library meant to be called from occam that serves as proof
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "occoids.h"
#include <stdio.h>
#include <string.h>

static int occoids_init = 0;
static cl_context *context;
static cl_device_id *device;
static cl_program prog;
static cl_command_queue *cq;
static cl_kernel k_occoids;

/**
 * Maps *w into something we can use in C
 * w[0] = size
 * w[1] = float
 * w[2] = int (size)
 */
void _occoids (int *w)
{
#if 0
  //fprintf (stdout, "x : %f, y : %f, %f, %f, %f, %f\n", (float) w[0], (float) w[1], (float) w[2], (float) w[3], (float) w[4], (float) w[5]);
  int i;
  for (i=0; i < 10; i++) {
    fprintf (stdout, "%f, ", (float) w[i]);
  }
  fprintf (stdout, "\n");
  vector velocity = {1.0, 1.0};
  memcpy (&velocity, w, sizeof(vector));
  fprintf (stdout, "velocity = %f, %f", velocity.x, velocity.y);
#else
  // in array
  int i;
  int arrsize = w[1];
  agentinfo *ai = w[0];
  //agentinfo *ai = arr[0];
  for (i=0; i<arrsize; i++) {
    //vector *pos = ai->position;
    //vector *vel = ai->velocity;
    fprintf (stdout, "localid = %d, type = %d, pos.x = %f, pos.y = %f, vel.x = %f, vel.y = %f, radius = %f, colour = %d\n", ai->localid, ai->type, ai->position.x, ai->position.y, ai->velocity.x, ai->velocity.y, ai->radius, ai->colour);
    // go to the next ai struct
    ai++;
  }

  // individual velocity
  vector *velocity = w[2];
  fprintf (stdout, "x = %f, y = %f\n", velocity->x, velocity->y);
#endif
#if CLOCCOIDS
  // need to get rid of the struct
  //occoids ();
#else
  //occoids_c ();
#endif
}

void _initoccoids (int *w)
{
  init_occoids ();
}

int occoids_c (agentinfo ai, vector accel)
{

  return 0;
}

int occoids (agentinfo ai, vector accel)
{
#if 0

  cl_int error;

#if ERROR_CHECK
  if (prog == NULL) {
    init_occoids ();
  }
#endif

  // Allocate memory for the kernel to work with
  cl_mem mem1, mem2;
  mem1 = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizeof(cl_char)*(width*2), 0, &error);
//  mem1 = clCreateBuffer(*context, CL_MEM_USE_HOST_PTR, sizeof(cl_char)*(width*2), data, &error);
  mem2 = clCreateBuffer(*context, CL_MEM_COPY_HOST_PTR, sizeof(cl_float)*5, job, &error);

  // get a handle and map parameters for the kernel
  error = clSetKernelArg(k_occoids, 0, sizeof(cl_mem), &mem1);
  error = clSetKernelArg(k_occoids, 1, sizeof(cl_mem), &mem2);

  // Perform the operation (width is 100 in this example)
  size_t worksize = width;
  error = clEnqueueNDRangeKernel(*cq, k_occoids, 1, NULL, &worksize, 0, 0, 0, 0);
  // Read the result back into data
  error = clEnqueueReadBuffer(*cq, mem1, CL_TRUE, 0, (size_t) (width*2), data, 0, 0, 0);

  // cleanup
  error = clFlush(*cq);
  clReleaseMemObject(mem1);
  clReleaseMemObject(mem2);

  if (error) {
    fprintf (stderr, "ERROR! : %s\n", errorMessageCL(error));
    exit(10);
  }

  return error;
#endif
  return 0;
}

cl_int print_occoids_kernel_info ()
{
  cl_int error = CL_SUCCESS;
  if (occoids_init) {
    size_t grp_size;
    error = clGetKernelWorkGroupInfo(k_occoids, *device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &grp_size, NULL);
    cl_ulong local_mem_size;
    error = clGetKernelWorkGroupInfo(k_occoids, *device, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &local_mem_size, NULL);
  }
  return error;
}

int init_occoids ()
{
  cl_int error;

  if (occoids_init)
    return 1;

  context = get_cl_context();
  device = get_cl_device();

  FILE *fp = fopen("occoids.cl", "r");
  if (!fp) {
    fprintf(stderr, "Failed to load kernel.\n");
    return(1);
  }
  char *src = (char*) malloc (MAX_SOURCE_SIZE);
  size_t srcsize = fread (src, 1, MAX_SOURCE_SIZE, fp);
  fclose (fp);
  const char *srcptr[]={src};

  // build CL program
  error = buildcl (srcptr, &srcsize, &prog, "");
  // create kernel
  k_occoids = clCreateKernel(prog, "occoids", &error);
  // get the shared CQ
  cq = get_command_queue();

  if (!error)
    occoids_init = 1;

  return error;
}
