/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "colour.h"

static int colours[6] = {31, 32, 33, 36, 35, 34};

void textcolour(int attr)
{
  char command[13];

  /* Command is the control command to the terminal */
  sprintf(command, "%c[%dm", 0x1B, colours[attr]);
  fprintf(stderr, "%s", command);
}
