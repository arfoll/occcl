/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include <occcl.h>

void _initialisecl(int *ws);
cl_int initialisecl();
cl_int buildcl(const char *srcptr[], size_t *srcsize, cl_program *prog);
const char* errorMessageCL(cl_int error);
cl_int getDevInfo();
cl_context* get_cl_context();
cl_device_id* get_cl_device();

