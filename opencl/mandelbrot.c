/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "mandelbrot.h"

static int mandelbrot_init = 0;
static cl_context *context;
static cl_device_id *device;
static cl_program prog;
static cl_kernel k_mandelbrot;
static cl_command_queue *cq;

cl_int table_int[] = { 32, 46, 44, 42, 126, 42, 94, 58, 59, 124, 38, 91, 36, 37, 64, 35 };

void mandelbrot_c (cl_char (*data)[200], cl_fract *job)
{
  int j, i;

  cl_fract y = job[0]/job[1] - job[2];

  for (j = 0; j < IMAGEHEIGHT; j++) {
    // calculate job[0] value
    *job = (cl_fract) j - (IMAGEHEIGHT/2);

#if DEBUG
    fprintf (stderr, "native job0 = %f, job1 = %f, job2, %f, job3 %f, y = %f\n", job[0], job[1], job[2], job[3], y);
#endif
    for (i = 0; i < IMAGEWIDTH; i++) {
      cl_fract real = ((i - IMAGEHEIGHT) / (job[1] * 2.0)) - job[3];
      cl_fract imag = y;
      cl_fract iter_real = 0.0;
      cl_fract iter_imag = 0.0;
      int count = 0;
      while ((((iter_real*iter_real)+(iter_imag*iter_imag)) < 32.0) && (count < 240)) {
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

void _initmandelbrot (int *w)
{
  init_mandelbrot();
}

int mandelbrot (cl_char (*data)[200], cl_fract *job) {
  
  cl_int error;

#if ERROR_CHECK
  if (prog == NULL) {
    init_mandelbrot();
  }
#endif

#if DEBUG
  fprintf (stderr, "opencl job0 = %f, job1 = %f, job2, %f, job3 %f\n", 
           job[0], job[1], job[2], job[3]);
#endif

#if 1 
  // test initialise data
  memset (data, 2,(IMAGEHEIGHT*IMAGEWIDTH*2)*sizeof(cl_char));
#endif

  // Allocate memory for the kernel to work with
  cl_mem mem1, mem2;
  mem1 = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizeof(cl_char)*(IMAGEHEIGHT*IMAGEWIDTH*2), 0, &error);
  mem2 = clCreateBuffer(*context, CL_MEM_COPY_HOST_PTR, sizeof(cl_fract)*5, job, &error);
  
  // get a handle and map parameters for the kernel
  error = clSetKernelArg(k_mandelbrot, 0, sizeof(cl_mem), &mem1);
  error = clSetKernelArg(k_mandelbrot, 1, sizeof(cl_mem), &mem2);

  // Perform the operation (width is 100 in this example)
  size_t worksize[3] = {IMAGEHEIGHT, IMAGEWIDTH, 0};
  error = clEnqueueNDRangeKernel(*cq, k_mandelbrot, 2, NULL, &worksize[0], 0, 0, 0, 0);
  // Read the result back into data
  error = clEnqueueReadBuffer(*cq, mem1, CL_TRUE, 0, (size_t) (IMAGEHEIGHT*IMAGEWIDTH*2), data, 0, 0, 0);

  // cleanup
  error = clFlush(*cq);
  clReleaseMemObject(mem1);
  clReleaseMemObject(mem2);

  if (error) {
    fprintf (stderr, "ERROR! : %s\n", errorMessageCL(error));
    exit(10);
  }

#if 0
  // this will print a frame coming out of the CL kernel in a dirty but functional manner
  // colours will be ignored
  int i,j;
  for (i=0; i < IMAGEHEIGHT; i++) {
    for (j=0; j < IMAGEWIDTH*2; j++) {
      if (j % 2) {
        fprintf (stderr, "%c", data[i][j]);
      }
    }
    fprintf(stderr, "\n");
  }

  exit (1);
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

int init_mandelbrot ()
{
  cl_int error;

  if (mandelbrot_init)
    return 1;

  context = get_cl_context();
  device = get_cl_device();

  FILE *fp = fopen("mandelbrot.cl", "r");
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
  k_mandelbrot = clCreateKernel(prog, "mandelbrot", &error);
  // get the shared CQ
  cq = get_command_queue();

  if (!error)
    mandelbrot_init = 1;

  return error;
}
