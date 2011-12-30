/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

__kernel void rot13 (__global const char* in, __global char* out )
{
    const uint index = get_global_id(0);
    char c=in[index];
    if (c<65 || c>122 || (c>90 && c<97)) {
        out[index] = in[index];
    } else {
        if (c>109 || (c>77 && c<97)) {
            out[index] = in[index]-13;
        } else {
            out[index] = in[index]+13;
        }
    }
}
