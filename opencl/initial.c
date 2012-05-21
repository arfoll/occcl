/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "initial.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERBOSE 0
#define DEBUG 1

static cl_platform_id platform;
static cl_device_id *device;
static cl_uint numdevices;
static cl_uint currentdevice = 0;
static cl_device_id devices[NUM_DEVICES];
static cl_command_queue cq[NUM_DEVICES];
static cl_context context[NUM_DEVICES];

/**
 * Occam-pi call for initialisecl
 */
void _initialisecl(int *ws)
{
  initialisecl(VERBOSE);
}

/**
 * Occam-pi call for destorycl
 */
void _destroycl(int *ws)
{
  destroycl();
}

/**
 * This initialisation is tested on AMDCCLE stream SDK (opencl 1.1)
 * on an AMD/ATI HD4850 graphics card on 32bit arch linux using linux 
 * kernel 3.0.3 with catalyst driver 
 * Also works on 32bit arch linux using kernel 3.2.x on cuda 4.x
 */
cl_int initialisecl(int verbose) 
{
#if ERROR_CHECK
  if (context == NULL) {
#endif
    int i;
    cl_int error;
    cl_uint platforms;
    device = &devices[currentdevice];

    //Fetch the Platform and Device IDs; we only want one.
    error = clGetPlatformIDs(1, &platform, &platforms);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Error getting platform ids: %s\n", errorMessageCL(error));
    }

    // prefer GPUs
    error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, NUM_DEVICES, device, &numdevices);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Couldn't get a CL_DEVICE_TYPE_GPU: %s\n", errorMessageCL(error));
    }

    if (verbose) {
      fprintf (stdout, "Found %d devices\n", numdevices);
    }

    // we don't have any GPU devices
    if (numdevices < 1) {
      fprintf(stderr, "Grabbing a CL_DEVICE_TYPE_ALL\n");
      error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, NUM_DEVICES, device, &numdevices);
      if (error != CL_SUCCESS) {
        fprintf(stderr, "Error getting device ids: %s\n", errorMessageCL(error));
      }
    }

    cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Error getting platform properties: %s\n", errorMessageCL(error));
    }

    // do this for every openCL device
    for (i=0; i<numdevices; i++) { 
      //AMD stream SDK requires the platform property
      context[i] = clCreateContext(properties, 1, &devices[i], NULL, NULL, &error);
      if (error != CL_SUCCESS) {
        fprintf(stderr, "Error creating context: %s\n", errorMessageCL(error));
      }

      //Create command queue
      cq[i] = clCreateCommandQueue(context[i], device[i], 0, &error);
      if (error) {
        fprintf (stderr, "ERROR creating command queue: %s\n", errorMessageCL(error));
      }
    }

    return error;
#if ERROR_CHECK
  }
  return CL_SUCCESS;
#endif
}

/**
 * Destroy opencl CQ and other shared objects
 */
cl_int destroycl()
{
  cl_int error;

  error = clReleaseCommandQueue(cq[currentdevice]);

  return error;
}

/**
 * Print the extensions supported by the device
 */
void printDevExt()
{
  char deviceExtensions[2048];
  clGetDeviceInfo((*device), CL_DEVICE_EXTENSIONS, sizeof(deviceExtensions), deviceExtensions, NULL);
  fprintf(stdout, "%s\n", deviceExtensions);
}

/**
 * Print device name
 */
void printDeviceName()
{
  char deviceName[1024];
  clGetDeviceInfo((*device), CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
  fprintf (stdout, "DEVICE NAME = %s\n",deviceName);
}

/**
 * Print general platform information
 */
void printPlatformInfo()
{
  char buffer[10240];
  clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 10240, buffer, NULL);
  printf("PROFILE = %s\n", buffer);
  clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 10240, buffer, NULL);
  printf("VERSION = %s\n", buffer);
  clGetPlatformInfo(platform, CL_PLATFORM_NAME, 10240, buffer, NULL);
  printf("NAME = %s\n", buffer);
  clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, 10240, buffer, NULL);
  printf("VENDOR = %s\n", buffer);
  clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, 10240, buffer, NULL);
  printf("EXTENSIONS = %s\n", buffer);
}

/**
 * Print Some device info
 */
void printDevInfo()
{
  // opencl device info
  int i;
  for (i = 0; i < numdevices; i++) {
    char buffer[10240];
    cl_uint buf_uint;
    cl_uint buf_uintarray[128];
    cl_ulong buf_ulong;
    printDeviceName();
    clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL);
    printf("DEVICE_VENDOR = %s\n", buffer);
    clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, sizeof(buffer), buffer, NULL);
    printf("DEVICE_VERSION = %s\n", buffer);
    clGetDeviceInfo(devices[i], CL_DRIVER_VERSION, sizeof(buffer), buffer, NULL);
    printf("DRIVER_VERSION = %s\n", buffer);
    clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(buf_uint), &buf_uint, NULL);
    printf("DEVICE_MAX_COMPUTE_UNITS = %u\n", (unsigned int)buf_uint);
    clGetDeviceInfo(devices[i], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(buf_uint), &buf_uint, NULL);
    printf("DEVICE_MAX_CLOCK_FREQUENCY = %uMhz\n", (unsigned int)buf_uint);
    clGetDeviceInfo(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(buf_ulong), &buf_ulong, NULL);
    printf("DEVICE_GLOBAL_MEM_SIZE = %lluMB\n", (unsigned long long)buf_ulong / BYTES_IN_MB);
    clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(buf_uint), &buf_uint, NULL);
    printf("CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS = %u\n", (unsigned int)buf_uint);
    clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(cl_uint) * buf_uint, &buf_uintarray, NULL);
    printf("CL_DEVICE_MAX_WORK_ITEM_SIZES = { %u, %u, %u }\n", buf_uintarray[0], buf_uintarray[1], buf_uintarray[2]);
  }
}

/**
 * Check the device supports the extension
 */
int extSupported(char *ext)
{
  char deviceExtensions[2048];
  clGetDeviceInfo((*device), CL_DEVICE_EXTENSIONS, sizeof(deviceExtensions), deviceExtensions, NULL);
  if(!strstr(deviceExtensions, ext)) {
    return 0;
  }
  return 1;
}

/**
 * Use the next device and return the current device
 */
int nextDevice()
{
  currentdevice++;
  currentdevice %= numdevices;

  device = &devices[currentdevice];

  return currentdevice;
}

/**
 * Get the number of CL devices
 */
int getMaxDevices()
{
  return numdevices;
}

/**
 * Get the current CL device
 */
int getCurrentDevice()
{
  return currentdevice;
}

/**
 * builds the CL program from src and returns and return it
 */
cl_int buildcl(const char *srcptr[], size_t *srcsize, cl_program *prog, const char *options, cl_int num_progs)
{
  cl_int error;
  int i;

  for (i=0; i<num_progs; i++) {
    //Submit the source code of the rot13 kernel to OpenCL
    prog[i] = clCreateProgramWithSource(context[i], 1, srcptr, srcsize, &error);
    //and compile it (after this we could extract the compiled version)
    error = clBuildProgram(prog[i], 0, NULL, options, NULL, NULL);
  }

#if ERROR_CHECK 
  if (error != CL_SUCCESS) {
    cl_char *log = (cl_char*)malloc(32768);
    error = clGetProgramBuildInfo(*prog, *device, CL_PROGRAM_BUILD_LOG, 32768, log, NULL);
    fprintf(stderr, "** %s\n", log);
    fprintf(stdout, "error : %s\n", errorMessageCL(error));
  }
#endif

  return error;
}

#if 0
/**
 * Get cl kernel profiling info so that opencl kernel binary can be dumped
 */
cl_int getProfilingInfo()
{
  clGetEventProfilingInfo(event, 
}
#endif

/**
 *
 */
int getCorrectDevice(char *requiredExt)
{
  int devicenum = 1;
  while (!extSupported(requiredExt) && devicenum < getMaxDevices()) {
    nextDevice();
    printDeviceName();
    printDevExt();
    devicenum++;
  }

  if (extSupported(requiredExt)) {
    return CL_SUCCESS;
  } else {
    return 1;
  }
}

/**
 * Return the cl_context
 */
cl_context* get_cl_context()
{
  return &context[currentdevice];
}

/**
 * Return the cl_device
 */
cl_device_id* get_cl_device()
{
  return device;
}

/**
 * Return the CommandQueue
 */
cl_command_queue* get_command_queue()
{
  return &cq[currentdevice];
}

int getNumDevices()
{
  return numdevices;
}

/**
 * Returns a string depending on the error code
 */
const char* errorMessageCL(cl_int error)
{
    static const char* errorString[] = {
        "CL_SUCCESS",
        "CL_DEVICE_NOT_FOUND",
        "CL_DEVICE_NOT_AVAILABLE",
        "CL_COMPILER_NOT_AVAILABLE",
        "CL_MEM_OBJECT_ALLOCATION_FAILURE",
        "CL_OUT_OF_RESOURCES",
        "CL_OUT_OF_HOST_MEMORY",
        "CL_PROFILING_INFO_NOT_AVAILABLE",
        "CL_MEM_COPY_OVERLAP",
        "CL_IMAGE_FORMAT_MISMATCH",
        "CL_IMAGE_FORMAT_NOT_SUPPORTED",
        "CL_BUILD_PROGRAM_FAILURE",
        "CL_MAP_FAILURE",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "CL_INVALID_VALUE",
        "CL_INVALID_DEVICE_TYPE",
        "CL_INVALID_PLATFORM",
        "CL_INVALID_DEVICE",
        "CL_INVALID_CONTEXT",
        "CL_INVALID_QUEUE_PROPERTIES",
        "CL_INVALID_COMMAND_QUEUE",
        "CL_INVALID_HOST_PTR",
        "CL_INVALID_MEM_OBJECT",
        "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
        "CL_INVALID_IMAGE_SIZE",
        "CL_INVALID_SAMPLER",
        "CL_INVALID_BINARY",
        "CL_INVALID_BUILD_OPTIONS",
        "CL_INVALID_PROGRAM",
        "CL_INVALID_PROGRAM_EXECUTABLE",
        "CL_INVALID_KERNEL_NAME",
        "CL_INVALID_KERNEL_DEFINITION",
        "CL_INVALID_KERNEL",
        "CL_INVALID_ARG_INDEX",
        "CL_INVALID_ARG_VALUE",
        "CL_INVALID_ARG_SIZE",
        "CL_INVALID_KERNEL_ARGS",
        "CL_INVALID_WORK_DIMENSION",
        "CL_INVALID_WORK_GROUP_SIZE",
        "CL_INVALID_WORK_ITEM_SIZE",
        "CL_INVALID_GLOBAL_OFFSET",
        "CL_INVALID_EVENT_WAIT_LIST",
        "CL_INVALID_EVENT",
        "CL_INVALID_OPERATION",
        "CL_INVALID_GL_OBJECT",
        "CL_INVALID_BUFFER_SIZE",
        "CL_INVALID_MIP_LEVEL",
        "CL_INVALID_GLOBAL_WORK_SIZE",
    };

    const int errorCount = sizeof(errorString) / sizeof(errorString[0]);
    const int index = -error;

    return (index >= 0 && index < errorCount) ? errorString[index] : "";
}
