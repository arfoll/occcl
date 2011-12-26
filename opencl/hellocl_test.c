/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "sin.h"
#include "rot13.h"
#include "mandelbrot.h"
#include "modulo.h"
#include "occoids.h"

#define PRINT_SIN 0
#define PRINT_MANDEL 0

int mandelbrotTest(int verbose, int iterations) {
  cl_int error = CL_SUCCESS;

  fprintf (stdout, "========= MANDELBROT =========\n");
  error = init_mandelbrot();
  fprintf (stdout, "init errors = %s\n", errorMessageCL(error));
  print_mandelbrot_kernel_info();
  cl_fract job[4] = {-25.000000, 534.086426, -0.271229, 1.159260};
  cl_char *dataptr = (cl_char*)malloc(sizeof(cl_char)*100*2*50);
  //memset(dataptr, 2, (sizeof(cl_char)*100*2*50));
  cl_char (*chdata)[200] = (cl_char*) dataptr;

  mandelbrot(chdata, &job[0]);

  free (dataptr);
  return error;
}

int occoidsTest() {
  cl_int error = CL_SUCCESS;

  fprintf (stdout, "========= OCCOIDS TEST =========\n");
}

int moduloTest() {
  cl_int error = CL_SUCCESS;

  fprintf (stdout, "========= MODULO PRECISION TEST =========\n");
  error = init_modulo();
  cl_int num = -30;
  cl_float amount = 0.123145;
  cl_float mod = 0;
  error += modulo(&num, &mod, &amount);
  fprintf (stdout, "CL exceution = %f\n", mod);
  cl_float res = (num - 50) / (amount * 2.0);
  res = (res * res) * res;
  fprintf (stdout, "C  execution = %f\n", res);
  return error;
}

int sinTest() {
  cl_int error = CL_SUCCESS;
  int i;

  fprintf (stdout, "========= SIN =========\n");
  // Create and initialize the input data
  cl_float *data;
  data = (cl_float*)malloc(sizeof(cl_float)*10240);
  for (i=0; i<10240; i++) {
    data[i] = i;
  }
  // init sin
  error = init_sin();
  fprintf (stdout, "init errors = %s\n", errorMessageCL(error));
  // run sin kernel
  error += clsin(data);
  fprintf (stdout, "clsin errors = %s\n", errorMessageCL(error));
#if PRINT_SIN
  for (i=0; i<10240; i++) {
    printf("sin(%d) = %f\n", i, data[i]);
    i = i + 100;
  }
#endif
  // free data
  free (data);

  return error;
}

int rot13Test() {
  cl_int error = CL_SUCCESS;

  fprintf (stdout, "========= ROT13 =========\n");
  // rot13 initialisation
  error = init_rot13();
  fprintf (stdout, "init errors = %s\n", errorMessageCL(error));

  char plaintext[]="Hello, World!";
  char ciphertext[sizeof plaintext];

  // run rot13 CL kernel
  error += rot13(plaintext, ciphertext);

  // Finally, output out happy message.
  fprintf (stdout, "rot13 errors = %d, ciphertext = %s\n", error, ciphertext);

  return error;
}

