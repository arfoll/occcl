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
  agentinfo *ai = w[0];
  cl_int arrsize = w[1];
  vector *velocity = w[2];
#endif
#if CLOCCOIDS
  occoids (ai, velocity, &arrsize);
  //printf("opencl res = %f, %f\n", velocity->x, velocity->y);
#else
  occoids_c (ai, velocity, arrsize);
  //printf("C res = %f, %f\n", ve2.x, ve2.y);
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
 * for now this emulates the start of the filter.infos() PROC and returns
 * n.boids and n.obstacles
 */
int occoids_c (agentinfo *ai, vector *velocity, cl_int size)
{
  int i;
  agentinfo *infos;
  vector accel;
  accel.x = 0.0;
  accel.y = 0.0;

  // TODO: small rule to do the can.see and make stuff = -1 when it's just there but not visible etc...

  //** centre of mass rule
  infos = ai;
  int count = 0;
  vector com;
  com.x = 0.0;
  com.y = 0.0;
  for (i=0; i<size; i++) {
    com.x = com.x + infos->position.x;
    com.y = com.y + infos->position.y;
    count++;
    infos++;
  }
  // don't do this if there where no agents seen
  if (count > 0) {
    com.x = com.x / count;
    com.y = com.y / count;
  }
  com.x = com.x / CENTREOFMASSFRACT;
  com.y = com.y / CENTREOFMASSFRACT;
  accel.x = com.x + accel.x;
  accel.y = com.y + accel.y;

  //** repulsion rule
  infos = ai;
  vector push;
  push.x = 0.0;
  push.y = 0.0;
  for (i=0; i<size; i++) {
    if (infos->type == ATBOID) {
      if (magnitute2(&infos->position) < (REPULSIONDIST * REPULSIONDIST)) {
        push.x = push.x - infos->position.x;
        push.y = push.y - infos->position.y;
      }
    }
    infos++;
  }
  push.x = push.x / REPULSIONFRACT;
  push.y = push.y / REPULSIONFRACT;
  accel.x = push.x + accel.x;
  accel.y = push.y + accel.y;

  //** mean velocity rule
  infos = ai;
  count = 0;
  vector pvel;
  pvel.x = 0.0;
  pvel.y = 0.0;
  for (i=0; i<size; i++) {
    pvel.x = pvel.x + infos->velocity.x;
    pvel.y = pvel.y + infos->velocity.y;
    count++;
    infos++;
  }
  // don't do this if there where no agents seen
  if (count > 0) {
    pvel.x = pvel.x / count;
    pvel.y = pvel.y / count;
  }
  pvel.x = pvel.x - velocity->x;
  pvel.y = pvel.y - velocity->y;
  pvel.x = pvel.x / MEANVELFRACT;
  pvel.y = pvel.y / MEANVELFRACT;
  accel.x = pvel.x + accel.x;
  accel.y = pvel.y + accel.y;

  //** obstacle rule
  push.x = 0.0;
  push.y = 0.0;
  infos = ai;
  for (i=0; i<size; i++) {
    if (infos->type == ATCYLINDER) {
      cl_float dist = sqrt (magnitute2(&infos->position)) - infos->radius;
      if (dist < 0.0) {
        push.x = push.x - infos->position.x;
        push.y = push.y - infos->position.y;
      }
      else if (dist < SOFTTHRESHOLD) {
        dist = 1.0 - (dist / SOFTTHRESHOLD);
        infos->position.x = infos->position.x * dist;
        infos->position.y = infos->position.y * dist;
        push.x = push.x - infos->position.x;
        push.y = push.y - infos->position.y;
      }
    }
    infos++;
  }
  push.x = push.x / OBSTACLEFRACT;
  push.y = push.y / OBSTACLEFRACT;
  accel.x = push.x + accel.x;
  accel.y = push.y + accel.y;


  //** accelerate
  velocity->x = velocity->x + (accel.x / SMOOTHACCEL);
  velocity->y = velocity->y + (accel.y / SMOOTHACCEL);
  
  if (abs(velocity->x) < 0.00000) {
    velocity->x = 0.0;
  }
  if (abs(velocity->y) < 0.00000) {
    velocity->y = 0.0;
  }

  cl_float mag = magnitute2 (velocity);
  if (mag > SPEEDLIMIT2) {
    cl_float div = mag/SPEEDLIMIT2;
    velocity->x = velocity->x / div;
    velocity->y = velocity->y / div;
  }
#if DEBUG  
  printf ("OCC velocity - %f - %f // mag = %f // limit2 = %f\n", velocity->x, velocity->y, mag, SPEEDLIMIT2);
#endif

  return 0;
}

int occoids (agentinfo *ai, vector *velocity, cl_int *size)
{
  cl_int error;

#if ERROR_CHECK
  if (prog == NULL) {
    init_occoids ();
  }
#endif

  // Allocate memory for the kernel to work with
  cl_mem mem1, mem2, mem3;
  mem1 = clCreateBuffer(*context, CL_MEM_COPY_HOST_PTR, sizeof(agentinfo)*(*size), ai, &error);
  mem2 = clCreateBuffer(*context, CL_MEM_COPY_HOST_PTR, sizeof(vector), velocity, &error);
  mem3 = clCreateBuffer(*context, CL_MEM_COPY_HOST_PTR, sizeof(cl_int), &size, &error);

  // write the arguments to memory
//  error = clEnqueueWriteBuffer(*cq, mem3, CL_TRUE, 0, sizeof(cl_int), size, 0, NULL, NULL);

  // get a handle and map parameters for the kernel
  error = clSetKernelArg(k_occoids, 0, sizeof(mem1), &mem1);
  error = clSetKernelArg(k_occoids, 1, sizeof(mem2), &mem2);
  error = clSetKernelArg(k_occoids, 2, sizeof(mem3), &mem3);

  // Perform the operation, there is only work item in this case
  size_t worksize = 1;
  error = clEnqueueNDRangeKernel(*cq, k_occoids, 1, NULL, &worksize, 0, 0, 0, 0);
  error = clEnqueueReadBuffer(*cq, mem2, CL_TRUE, 0, sizeof(vector), velocity, 0, NULL, NULL);

  // cleanup
  // TODO: clFlush is NOT optimal
  //clFlush(*cq);
  clReleaseMemObject(mem1);
  clReleaseMemObject(mem2);
  clReleaseMemObject(mem3);

  if (error) {
    fprintf (stderr, "ERROR! : %s\n", errorMessageCL(error));
    exit(10);
  }

  return error;
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
  error = buildcl (srcptr, &srcsize, &prog, "", NUM_GPUS);
  // create kernel
  k_occoids = clCreateKernel(prog, "occoids", &error);
  // get the shared CQ
  cq = get_command_queue();

  if (!error)
    occoids_init = 1;

  printf("occoids init over!\n");

  return error;
}
