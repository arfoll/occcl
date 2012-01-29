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
 */
void _occoids (int *w)
{
#if DEBUG
  int i;
  int arrsize = w[1];
  agentinfo *ai = w[0];
  vector *velocity = w[2];
  for (i=0; i<arrsize; i++) {
    //vector *pos = ai->position;
    //vector *vel = ai->velocity;
    fprintf (stderr, "localid = %d, type = %d, pos.x = %f, pos.y = %f, " \
             "vel.x = %f, vel.y = %f, radius = %f, colour = %d, x = %f, y = %f\n",
             ai->localid, ai->type, ai->position.x, ai->position.y,
             ai->velocity.x, ai->velocity.y, ai->radius, ai->colour,
             velocity->x, velocity->y);
    // go to the next ai struct
    ai++;
  }
#else
  int arrsize = w[1];
  agentinfo *ai = w[0];
  vector *velocity = w[2];
#endif
#if CLOCCOIDS
  // need to get rid of the struct
  //occoids ();
#else
  occoids_c (ai, velocity, arrsize);
#endif
}

void _initoccoids (int *w)
{
  init_occoids ();
}

/**
 * Returns the magnitude of a vector
 */
cl_float magnitute2 (vector *ve)
{
  return (ve->x * ve->x) + (ve->y * ve->y);
}

/**
 * function emulates the can.see function in occam
 */
int cansee (agentinfo *info, vector *velocity)
{
  #define TRUE 0
  #define FALSE 1
  #define ATCYLINDER 2
  #define VISIONRADIUS 0.25
  #define VISIONANGLE 200.0

  if (magnitute2(&info->position) > (VISIONRADIUS*VISIONANGLE)) {
    return FALSE;
  }
  else if (info->type == ATCYLINDER) {
    return TRUE;
  }
  else if (magnitute2(velocity) < 0.00000) {
    return TRUE;
  }
#if 0  
  elif () {
    return FALSE;
  }
#endif
  else {
    return TRUE;
  }
}

/**
 * for now this emulates the start of the filter.infos() PROC and returns
 * n.boids and n.obstacles
 * TODO: the rest of the function
 */
int occoids_c (agentinfo *ai, vector *accel, int size)
{
  int i;
  int nboids = 0;
  int nobstacles = 0;
  for (i=0; i<size; i++) {
    if (cansee(ai, accel)) {
      if (ai->type == 1)
        *nboids++;
      else if (ai->type == 2)
        *nobstacles++;
    }
    ai++;
  }

  return 0;
}

int occoids (agentinfo *ai, vector *accel, int size)
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
