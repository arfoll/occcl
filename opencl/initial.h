/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#ifndef INITIAL_H
#define INITIAL_H

#include <occcl.h>

// Defines the maximum number of CL devices that will be recognised by occcl
#define NUM_DEVICES 8 
// Number of bytes in a MB - used by printDevInfo
#define BYTES_IN_MB 1048576
// Biggest build line possible for opencl - not a real limit!
#define MAX_BUILD_LINE_LENGTH 1024

cl_int initialisecl(int verbose);
cl_int destroycl();
cl_int buildcl(const char *srcptr[], size_t *srcsize, cl_program *prog, const char *options, int num_progs);
const char* errorMessageCL(cl_int error);
void printDevExt();
void printDeviceName();
void printPlatformInfo();
void printDevInfo();
int extSupported(char *ext);
int nextDevice();
int getMaxDevices();
int getCorrectDevice(char *requiredExt);
int getNumDevices();
cl_context* get_cl_context();
cl_device_id* get_cl_device();
cl_command_queue* get_command_queue();

#endif
