/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - bl73@kent.ac.uk
 */

#include <occcl.h>

cl_int initialisecl(cl_context *context, cl_device_id *device);
cl_int buildcl(const char *srcptr[], size_t *srcsize, cl_context *context, cl_program *prog);
const char* errorMessageCL(cl_int error);

