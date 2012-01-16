/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "mandelbrot.h"

static int mandelbrot_init = 0;
static int mandelbrot_cl_float;
static cl_context *context;
static cl_device_id *device;
static cl_program prog;
static cl_kernel k_mandelbrot;
static cl_command_queue *cq;

static cl_int table_int[] = { 32, 46, 44, 42, 126, 42, 94, 58, 59, 124, 38, 91, 36, 37, 64, 35 };

// TODO: split jobs into [850][5] array where [x][4] is y
#define NUM_JOBS 4250
cl_fract jobs[NUM_JOBS];

void _mandelbrot (int *w)
{ 
  cl_char (*data)[IMAGEHEIGHT][IMAGEWIDTH*2] = (cl_char*) w[0];
  // due to the [][][] array w[1] is 850, w[2] is 50, w[3] is 300
  mandelbrot (data);
}

void _initmandelbrot (int *w)
{
  init_mandelbrot();
}

int mandelbrot (cl_char (*data)[50][200])
{  
  cl_int error;
  int i;

#if ERROR_CHECK
  if (prog == NULL) {
    init_mandelbrot();
  }
#endif

  // Allocate memory for the kernel to work with
  cl_mem mem1, mem2;
  mem1 = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizeof(cl_char)*(NFRAMES*IMAGEHEIGHT*IMAGEWIDTH*2), 0, &error);
  mem2 = clCreateBuffer(*context, CL_MEM_COPY_HOST_PTR, sizeof(cl_fract)*5*NFRAMES, jobs, &error);
  
  // get a handle and map parameters for the kernel
  error = clSetKernelArg(k_mandelbrot, 0, sizeof(cl_mem), &mem1);
  error = clSetKernelArg(k_mandelbrot, 1, sizeof(cl_mem), &mem2);

  // Perform the operation (width is 100 in this example)
  size_t worksize[3] = {NFRAMES, IMAGEWIDTH, IMAGEHEIGHT};
  error = clEnqueueNDRangeKernel(*cq, k_mandelbrot, 3, NULL, &worksize[0], 0, 0, 0, 0);
  // Read the result back into data
  error = clEnqueueReadBuffer(*cq, mem1, CL_TRUE, 0, (size_t) (NFRAMES*IMAGEHEIGHT*IMAGEWIDTH*2), data, 0, 0, 0);

  // cleanup - don't perform a flush as the queue is now shared between all executions. The
  // blocking clEnqueueReadBuffer should be enough
  clReleaseMemObject(mem1);
  clReleaseMemObject(mem2);

  if (error) {
    fprintf (stderr, "ERROR! : %s\n", errorMessageCL(error));
    exit(10);
  }

#if C_PRINT
  // this will print a frame coming out of the CL kernel in a dirty but functional manner
  int z, j;
  int colour = -1;
  for (z=0; z < NFRAMES; z++) {
    for (i=0; i < IMAGEHEIGHT; i++) {
      for (j=0; j < IMAGEWIDTH*2; j++) {
        if (colour != data[z][i][j]) {
          colour = data[z][i][j];
          textcolour(colour);
        }
        j++;
        fprintf (stdout, "%c", data[z][i][j]);
      }
      fprintf(stdout, "\n");
    }
  }
#endif
 
  return error;
}

cl_int print_mandelbrot_kernel_info ()
{
  cl_int error = CL_SUCCESS;
  if (mandelbrot_init) {
    size_t grp_size;
    error = clGetKernelWorkGroupInfo(k_mandelbrot, *device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &grp_size, NULL);
    cl_ulong local_mem_size;
    error = clGetKernelWorkGroupInfo(k_mandelbrot, *device, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &local_mem_size, NULL);
#if 0
    size_t[3] build_grp_size;
    error = clGetKernelWorkGroupInfo(k_mandelbrot, *device, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t)*3, &build_grp_size, NULL);
    fprintf (stdout, "Group Size is %lu. Local Mem Size is %lu. Compile Group Size is %lu, %lu, %lu\n", (unsigned long) grp_size, (unsigned long) local_mem_size, build_grp_size[0], build_grp_size[1], build_grp_size[2]);
#endif
  }
  return error;
}

/**
 * Inialises the jobs array
 */
void initialiseJobs()
{
  int i, index;

  cl_fract zoom = 16.0;
  cl_fract xdrift = 0.0;
  cl_fract ydrift = 0.0;
  cl_fract diffx;
  cl_fract diffy;
  cl_fract xtarget = 1.16000014859;
  cl_fract ytarget = -0.27140215303;

  for (i = 0; i < 850; i++) {
    index = i*5;
    //jobs[0] = (frameid - idx)
    jobs[index] = i; 
    jobs[index+1] = zoom;
    jobs[index+2] = ydrift;
    jobs[index+3] = xdrift;
//    jobs[index+4] = jobs[index]/job[index+1] - job[index+2];

    zoom = zoom + (zoom / 32.0);
    diffx = xtarget - xdrift;
    diffy = ytarget - ydrift;
    xdrift = xdrift + (diffx / 16.0);
    ydrift = ydrift + (diffy / 16.0);
  }
}

int init_mandelbrot ()
{
  cl_int error;

  if (mandelbrot_init)
    return 1;

  context = get_cl_context();
  device = get_cl_device();

  FILE *fp;
  fp = fopen("mandelbrot.cl", "r");
  if (!fp) {
    fprintf(stderr, "Failed to load kernel.\n");
    return(1);
  }
  char *src = (char*) malloc (MAX_SOURCE_SIZE);
  size_t srcsize = fread (src, 1, MAX_SOURCE_SIZE, fp);
  fclose (fp);
  const char *srcptr[]={src};

  // build CL program with a USE_DOUBLE define if we found the correct extension
  if (getCorrectDevice("cl_khr_fp64") == CL_SUCCESS) {
    error = buildcl (srcptr, &srcsize, &prog, "-D USE_DOUBLE -cl-fast-relaxed-math -cl-mad-enable");
  }
  else {
    mandelbrot_cl_float = 1;
    error = buildcl (srcptr, &srcsize, &prog, "-D USE_FLOAT -cl-fast-relaxed-math -cl-mad-enable");
  }
  // create kernel
  k_mandelbrot = clCreateKernel(prog, "mandelbrot", &error);
  // get the shared CQ
  cq = get_command_queue();

  initialiseJobs();

  if (!error)
    mandelbrot_init = 1;

  return error;
}
