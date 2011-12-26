/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "occoids.h"

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
#if CLOCCOIDS
  // need to get rid of the struct
  occoids ();
#else
  int size = (int) w[0];
  if (size != 1 && size != 0) {
    fprintf (stderr, "size is %d\n", size);
    fprintf (stderr, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", (int) w[1], (int) w[2], (int) w[3], (int) w[4], (int) w[5], (int) w[6], (int) w[7], (int) w[8], (int) w[9], (int) w[10]);
    fprintf (stderr, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", (float) w[1], (float) w[2], (float) w[3], (float) w[4], (float) w[5], (float) w[6], (float) w[7], (float) w[8], (float) w[9], (float) w[10]);
    exit(0);
  }
  //occoids_c ();
#endif
}

void _initoccoids (int *w)
{
  init_occoids ();
}

int occoids_c (struct agentinfo ai, struct vector accel)
{

  return 0;
}

int occoids (struct agentinfo ai, struct vector accel)
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
