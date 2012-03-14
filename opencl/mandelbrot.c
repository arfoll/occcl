/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "mandelbrot.h"

#define MAX_GPUS 4

static int mandelbrot_init = 0;
static int mandelbrotvis_init = 0;
static int mandelbrot_cl_float;
static cl_context *context;
static cl_device_id *device;
static cl_program prog;
static cl_program progvis[MAX_GPUS];
static cl_kernel k_mandelbrot;
static cl_kernel k_mandelbrotvis[MAX_GPUS];
static cl_command_queue *cq;
static int numdevices = 1;

static int visheight = 240;
static int viswidth = 320;
static int framesperworker = 1;
static int iterations = 1500;
// our max iterations is 3000
cl_fract jobs[3000*3];

cl_int table_int[] = { 32, 46, 44, 42, 126, 42, 94, 58, 59, 124, 38, 91, 36, 37, 64, 35 };

void mandelbrot_c (cl_char (*data)[200], cl_fract *job)
{
  int j, i;

  for (j = 0; j < IMAGEHEIGHT; j++) {
    // calculate job[0] value
    job[0] = (cl_fract) j - (IMAGEHEIGHT/2);
    cl_fract y = job[0]/job[1] - job[2];

#if DEBUG
    fprintf (stderr, "native job0 = %f, job1 = %f, job2, %f, job3 %f, y = %f\n", job[0], job[1], job[2], job[3], y);
#endif
    for (i = 0; i < IMAGEWIDTH; i++) {
      cl_fract real = ((i - IMAGEHEIGHT) / (job[1] * 2.0)) - job[3];
      cl_fract imag = y;
      cl_fract iter_real = 0.0;
      cl_fract iter_imag = 0.0;
      int count = 0;
      while ((((iter_real*iter_real)+(iter_imag*iter_imag)) < 32.0) && (count < visheight)) {
        cl_fract iter_real2 = iter_real;
        cl_fract iter_imag2 = iter_imag;
        cl_fract iter_r;
        cl_fract iter_i;
        iter_r = (iter_real*iter_real2) - (iter_imag*iter_imag2);
        iter_i = (iter_imag*iter_real2) + (iter_real*iter_imag2);
        iter_real = real + iter_r;
        iter_imag = imag + iter_i;
        count++;
      }
      int val = count % 16;
      data[j][i*2] = (char) (val % 6);
      data[j][(i*2)+1] = table_int[val];
    }
  }
}

void _mandelbrot (int *w)
{ 
  cl_char (*data)[200] = (cl_char*) w[0];
  // due to the [][] array w[1] is 50 and w[2] is 200
  // w[3] would be jobs
#if CLMANDEL
  mandelbrot (data, (cl_fract*) (w[3]));
#else
  mandelbrot_c (data, (cl_fract*) (w[3]));
#endif
}

void _mandelbrotvis (int *w)
{ 
  cl_int *data = (cl_int*) w[0];
  // pass the correct part of the jobs array
  cl_fract *jobsarr = &jobs[w[4]*JOBS_PER_FRAME];
  mandelbrotvis (data, (cl_fract*) jobsarr);
}

void _initmandelbrot (int *w)
{
  init_mandelbrot();
}

void _initmandelbrotvis (int *w)
{
  viswidth = (int) w[0];
  visheight = (int) w[1];
  framesperworker = (int) w[2];
  iterations = (int) w[3];
  //  printf("%d, %d, %d\n\n\n\n", viswidth, visheight, framesperworker);
  init_mandelbrotvis();
}

int mandelbrot (cl_char (*data)[200], cl_fract *job)
{  
  cl_int error;
  int i;

#if ERROR_CHECK
  if (prog == NULL) {
    init_mandelbrot();
  }
#endif

#if DEBUG
  fprintf (stderr, "opencl job0 = %f, job1 = %f, job2, %f, job3 %f\n", 
           job[0], job[1], job[2], job[3]);
#endif

#if 0 
  // test initialise data
  memset (data, 2,(IMAGEHEIGHT*IMAGEWIDTH*2)*sizeof(cl_char));
#endif

  // Allocate memory for the kernel to work with
  cl_mem mem1, mem2;
  mem1 = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizeof(cl_char)*(IMAGEHEIGHT*IMAGEWIDTH*2), 0, &error);

  if (mandelbrot_cl_float) {
    cl_float jobfloat[4];
    for (i=0; i<4; i++)
      jobfloat[i] = (cl_float) job[i];

    mem2 = clCreateBuffer(*context, CL_MEM_COPY_HOST_PTR, sizeof(cl_float)*4, jobfloat, &error);
  } else {
    mem2 = clCreateBuffer(*context, CL_MEM_COPY_HOST_PTR, sizeof(cl_fract)*4, job, &error);
  }
  
  // get a handle and map parameters for the kernel
  error = clSetKernelArg(k_mandelbrot, 0, sizeof(cl_mem), &mem1);
  error = clSetKernelArg(k_mandelbrot, 1, sizeof(cl_mem), &mem2);

  // Perform the operation (width is 100 in this example)
  size_t worksize[3] = {IMAGEHEIGHT, IMAGEWIDTH, 0};
  error = clEnqueueNDRangeKernel(*cq, k_mandelbrot, 2, NULL, &worksize[0], 0, 0, 0, 0);
  // Read the result back into data
  error = clEnqueueReadBuffer(*cq, mem1, CL_TRUE, 0, sizeof(cl_char)*(IMAGEHEIGHT*IMAGEWIDTH*2), data, 0, 0, 0);

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
  int j;
  int colour = -1;
  for (i=0; i < IMAGEHEIGHT; i++) {
    for (j=0; j < IMAGEWIDTH*2; j++) {
      if (colour != data[i][j]) {
        colour = data[i][j];
        textcolour(colour);
      }
      j++;
      fprintf (stdout, "%c", data[i][j]);
    }
    fprintf(stdout, "\n");
  }
#endif

  return error;
}

int mandelbrotvis (cl_int *data, cl_fract *job)
{  
  cl_int error;
  int i;

#if MULTI_GPUS 
  // move to new context/cq
  int currentdevice = nextDevice();
  cl_command_queue *cqm = get_command_queue();
  cl_context *cm = get_cl_context();
#endif

  // Allocate memory for the kernel to work with
  cl_mem mem1, mem2;
  mem1 = clCreateBuffer(*cm, CL_MEM_WRITE_ONLY, sizeof(cl_int)*(visheight*viswidth*framesperworker), 0, &error);

  if (mandelbrot_cl_float) {
    cl_float jobfloat[framesperworker*JOBS_PER_FRAME];
    for (i=0; i<framesperworker*JOBS_PER_FRAME; i++)
      jobfloat[i] = (cl_float) job[i];

    mem2 = clCreateBuffer(*cm, CL_MEM_COPY_HOST_PTR, sizeof(cl_float)*framesperworker*JOBS_PER_FRAME, jobfloat, &error);
  } else {
    mem2 = clCreateBuffer(*cm, CL_MEM_COPY_HOST_PTR, sizeof(cl_fract)*framesperworker*JOBS_PER_FRAME, job, &error);
  }
  
  // get a handle and map parameters for the kernel
  error = clSetKernelArg(k_mandelbrotvis[currentdevice], 0, sizeof(mem1), &mem1);
  error = clSetKernelArg(k_mandelbrotvis[currentdevice], 1, sizeof(mem2), &mem2);

  size_t worksize[3] = {visheight, viswidth, framesperworker};
  error = clEnqueueNDRangeKernel(*cqm, k_mandelbrotvis[currentdevice], 3, NULL, &worksize[0], 0, 0, 0, 0);
  // Read the result back into data
  error = clEnqueueReadBuffer(*cqm, mem1, CL_TRUE, 0, sizeof(cl_int)*(visheight*viswidth*framesperworker), data, 0, 0, 0);

  // cleanup - don't perform a flush as the queue is now shared between all executions. The
  // blocking clEnqueueReadBuffer should be enough
  clReleaseMemObject(mem1);
  clReleaseMemObject(mem2);

  if (error) {
    fprintf (stderr, "ERROR! : %s\n", errorMessageCL(error));
    exit(10);
  }

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

int init_mandelbrot ()
{
  cl_int error;

  if (mandelbrot_init)
    return 1;

  context = get_cl_context();
  device = get_cl_device();

  FILE *fp;
  fp = fopen(CLKERNELDEFS, "r");
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
    error = buildcl (srcptr, &srcsize, &prog, "-D USE_DOUBLE -cl-fast-relaxed-math -cl-mad-enable", NUM_GPUS);
  }
  else {
    mandelbrot_cl_float = 1;
    error = buildcl (srcptr, &srcsize, &prog, "-D USE_FLOAT -cl-fast-relaxed-math -cl-mad-enable", NUM_GPUS);
  }
  // create kernel
  k_mandelbrot = clCreateKernel(prog, "mandelbrot", &error);
  // get the shared CQ
  cq = get_command_queue();

  if (!error)
    mandelbrot_init = 1;

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
  cl_fract ytarget = -0.27140215302;

  for (i = 0; i < iterations; i++) {
    index = i*JOBS_PER_FRAME;
    jobs[index] = zoom;
    jobs[index+1] = ydrift;
    jobs[index+2] = xdrift;

    fprintf (stderr, "%f, %f, %f\n", jobs[index], jobs[index+1], jobs[index+2]);

    zoom = zoom + (zoom / 32.0);
    diffx = xtarget - xdrift;
    diffy = ytarget - ydrift;
    xdrift = xdrift + (diffx / 16.0);
    ydrift = ydrift + (diffy / 16.0);
  }
}

int init_mandelbrotvis ()
{
  int i;
  cl_int error;

  if (mandelbrotvis_init)
    return 1;

  context = get_cl_context();
  device = get_cl_device();

  FILE *fp;
  fp = fopen(CLVISKERNELDEFS, "r");
  if (!fp) {
    fprintf(stderr, "Failed to load kernel.\n");
    return(1);
  }
  char *src = (char*) malloc (MAX_SOURCE_SIZE);
  size_t srcsize = fread (src, 1, MAX_SOURCE_SIZE, fp);
  fclose (fp);
  const char *srcptr[]={src};

  // get the number of GPUS/DEVICES available
  numdevices = getNumDevices();

  // build CL program with a USE_DOUBLE define if we found the correct extension
  char *precision = "             ";
  if (getCorrectDevice("cl_khr_fp64") == CL_SUCCESS) {
    precision = "-D USE_DOUBLE";
  }
  else {
    mandelbrot_cl_float = 1;
    precision = "-D USE_FLOAT";
  }

  char options[MAX_BUILD_LINE_LENGTH];
  // following options seem to speed things up a little
  char *compile_opt = "-cl-fast-relaxed-math -cl-mad-enable";
  snprintf(options, MAX_BUILD_LINE_LENGTH,
           "%s -D IMAGEWIDTHVIS=%d -D IMAGEHEIGHTVIS=%d %s", precision, viswidth, visheight, compile_opt);
  error = buildcl (srcptr, &srcsize, &progvis[0], options, numdevices);
//  printf("%s\n\n\n", options);

  // create kernel
  for (i=0; i<numdevices; i++) {
    k_mandelbrotvis[i] = clCreateKernel(progvis[i], "mandelbrot_vis", &error);
  }
  // get the shared CQ
  cq = get_command_queue();

  // initialise the jobs array
  initialiseJobs();

  if (!error)
    mandelbrotvis_init = 1;

  return error;
}
