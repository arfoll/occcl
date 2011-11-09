/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "sin.h"
#include "rot13.h"
#include "mandelbrot.h"
#include <stdlib.h>
#include <stdio.h>

#define SIN 1
#define PRINT_SIN 0
#define PRINT_MANDEL 1

int main() {
  char plaintext[]="Hello, World!";
  char ciphertext[sizeof plaintext];
  
  cl_int error;
#if SIN
  int i;
#endif

  // CL initialisation
  error = initialisecl();

  getDevInfo();

#if SIN
  // SIN wave
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
#endif

  // reinit cl_error
  error = 0;
  fprintf (stdout, "========= ROT13 =========\n");
  // rot13 initialisation
  error += init_rot13();
  fprintf (stdout, "init errors = %s\n", errorMessageCL(error));

  // run rot13 CL kernel
  error += rot13(plaintext, ciphertext);

  // Finally, output out happy message.
  fprintf (stdout, "rot13 errors = %d, ciphertext = %s\n", error, ciphertext);

  // reinit cl_error
  error = 0;
  fprintf (stdout, "========= MANDELBROT =========\n");

  // mandelbrot initialisation
  error += init_mandelbrot();
  fprintf (stdout, "init errors = %s\n", errorMessageCL(error));
  // run mandelbrot CL kernel
  cl_int width = 100;
  double *job = (double*)malloc(sizeof(double)*4);
  for (i=0; i < 4; i++)
    job[i] = 0.0;
  cl_char *chdata = (cl_char*)malloc(sizeof(cl_char)*width*2);
  for (i=0; i < width*2; i++)
    chdata[i] = i;
  double y = 4.59; 
  error += mandelbrot(chdata, job, width, &y);
  fprintf (stdout, "mandelbrot errors = %s\n", errorMessageCL(error));
#if PRINT_MANDEL
  for (i=0; i < width; i++) {
    printf("mandel(%d) = %d\n", i, (int) chdata[i]);
    i = i + 10;
  }
#endif


  return error;
}
