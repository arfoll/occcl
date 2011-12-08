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

#define PRINT_SIN 0
#define PRINT_MANDEL 0

int getCorrectDevice(char *requiredExt) {
  int devicenum = 1;
  while (!extSupported(requiredExt) && devicenum < getMaxDevices()) {
    nextDevice();
    fprintf (stdout, "========= CHANGED CL DEVICE =========\n");
    printDeviceName();
    printDevExt();
    devicenum++;
  }

  if (extSupported(requiredExt)) {
    return CL_SUCCESS;
  } else {
    fprintf (stdout, "no devices on this system support %s, which is a required extension\n", requiredExt);
    return 1;
  }
}

int mandelbrotTest(int verbose, int iterations) {
  cl_int error;
  int i;

  fprintf (stdout, "========= MANDELBROT =========\n");
  // mandelbrot initialisation
  error = init_mandelbrot();
  fprintf (stdout, "init errors = %s\n", errorMessageCL(error));
  // print some device info
  print_mandelbrot_kernel_info();
  // run mandelbrot CL kernel against C mandelbrot func
  cl_int width = 100;
  //make job array with rawdata
  cl_fract *job = (cl_fract*)malloc(sizeof(cl_fract)*5);
  cl_fract *job_ori = (cl_fract*)malloc(sizeof(cl_fract)*4);
  cl_fract rawdata[4] = {-25.000000, 534.086426, -0.271229, 1.159260};
  for (i=0; i < 4; i++) {
    job[i] = job_ori[i] = rawdata[i];
  } 
  // make char arrays and fill them with blank data
  cl_char *chdata = (cl_char*)malloc(sizeof(cl_char)*width*2);
  cl_char *chdata2 = (cl_char*)malloc(sizeof(cl_char)*width*2);
  for (i=0; i < width*2; i++) {
    chdata[i] = i;
    chdata2[i] = i;
  }

  // run the mandelbrot 50 times changing job[1] values
  int errors = 0;
  int x = 0;
  if (iterations == 8) {
    i = iterations;
  } else {
    i = -25;
  }
  for (; i < iterations+1; i++) {
    job[0] = job_ori[0] = i;
    // calculate y for cl function
    job[4] = job[0]/job[1] - job[2];
    error += mandelbrot(chdata, job, width);
    mandelbrot_c(chdata2, job_ori, width);

    // error checking
    for (x=0; x < width; x++) {
      if (chdata[x] != chdata2[x]) {
        errors++;
        if (verbose) {
          fprintf(stdout, "mandelcl(%d) = %d vs %d\n", x, (int) chdata[x], (int) chdata2[x]);
        }
      }
    }
  }

  // print any possible errors from CL kernel call
  fprintf (stdout, "mandelbrot CL errors * %d = %s\n", iterations, errorMessageCL(error));
  fprintf (stdout, "mandelbrot calculations with %d errors\n", errors);

  // cleanup
  free (chdata);
  free (chdata2);
  return error;
}

int moduloTest() {
  cl_int error;

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
  cl_int error;
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
  cl_int error;

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

