/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "initial.h"

#include <stdio.h>
#include <string.h>

#define DEBUG 1

static cl_context context;
static cl_device_id device;

void _initialisecl(int *ws)
{
  initialisecl();
}

/**
 * This initialisation is tested on AMDCCLE stream SDK (opencl 1.1)
 * on an AMD/ATI HD4850 graphics card on 32bit arch linux using linux 
 * kernel 3.0.3 with catalyst driver 
 */
cl_int initialisecl() 
{
#if ERROR_CHECK
  if (context == NULL) {
#endif
    cl_int error;
    cl_platform_id platform;
    cl_uint platforms, devices;

    //Fetch the Platform and Device IDs; we only want one.
    error = clGetPlatformIDs(1, &platform, &platforms);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Error getting platform ids: %s", errorMessageCL(error));
    }

    error=clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &devices);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Error getting device ids: %s", errorMessageCL(error));
    }

    cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Error getting platform properties: %s", errorMessageCL(error));
    }

    //AMD stream SDK requires the platform property
    context = clCreateContext(properties, 1, &device, NULL, NULL, &error);
    if (error != CL_SUCCESS) {
      fprintf(stderr, "Error creating context: %s", errorMessageCL(error));
    }

    return error;
#if ERROR_CHECK
  }

  return CL_SUCCESS;
#endif
}

/**
 * builds the CL program from src and returns and return it
 */
cl_int buildcl(const char *srcptr[], size_t *srcsize, cl_program *prog)
{
  cl_int error;
  //Submit the source code of the rot13 kernel to OpenCL
  *prog = clCreateProgramWithSource(context, 1, srcptr, srcsize, &error);
  //and compile it (after this we could extract the compiled version)
  error = clBuildProgram(*prog, 0, NULL, "", NULL, NULL);

// TODO: ERROR_CHECK not DEBUG
#if DEBUG
  if (error != CL_SUCCESS) {
    char log[4096];
    error = clGetProgramBuildInfo(*prog, device, CL_PROGRAM_BUILD_LOG, 4096, log, NULL);
    fprintf(stderr, "** %s\n", log);
    fprintf(stdout, "error : %s\n", errorMessageCL(error));
  }
#endif

  return error;
}

cl_context* get_cl_context()
{
  return &context;
}

cl_device_id* get_cl_device()
{
  return &device;
}

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
