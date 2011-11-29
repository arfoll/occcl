/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include <occcl.h>

#define NUM_DEVICES 2

void _initialisecl(int *ws);
cl_int initialisecl();
cl_int buildcl(const char *srcptr[], size_t *srcsize, cl_program *prog, const char *options);
const char* errorMessageCL(cl_int error);
void printDevExt();
void printDeviceName();
int extSupported(char *ext);
void nextDevice();
int getMaxDevices();
cl_context* get_cl_context();
cl_device_id* get_cl_device();

