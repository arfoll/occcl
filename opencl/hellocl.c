/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include "sin.h"
#include "rot13.h"
#include "mandelbrot.h"
#include "modulo.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define DEVCHECK 1
#define SIN 1
#define PRINT_SIN 0
#define PRINT_MANDEL 0

int main() {
  char plaintext[]="Hello, World!";
  char ciphertext[sizeof plaintext];
  
  cl_int error;
#if SIN
  int i;
#endif

  // CL initialisation
  error = initialisecl();

  printDeviceName();
  printDevExt();
#if DEVCHECK
  int devicenum = 0;
  while (!extSupported("cl_khr_byte_addressable_store") && devicenum < getMaxDevices()) {
    nextDevice();
    printDeviceName();
    printDevExt();
    devicenum++;
  }

  if (devicenum > getMaxDevices()) {
    fprintf (stdout, "no devices on this system support the required extension");
    return 0;
  }
#endif

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
  cl_fract *job = (cl_fract*)malloc(sizeof(cl_fract)*5);
  cl_fract *job2 = (cl_fract*)malloc(sizeof(cl_fract)*4);
  cl_fract rawdata[4] = {-25.000000, 17.547363, -0.047774, 0.204189};
  for (i=0; i < 4; i++) {
    job[i]  = rawdata[i];
    job2[i] = rawdata[i];
  }
  cl_fract y = job[0]/job[1] - job[2];
  job[4] = y;
  cl_char *chdata = (cl_char*)malloc(sizeof(cl_char)*width*2);
  cl_char *chdata2 = (cl_char*)malloc(sizeof(cl_char)*width*2);
  for (i=0; i < width*2; i++) {
    chdata[i] = i;
    chdata2[i] = i;
  }
  error += mandelbrot(chdata, job, width);
  mandelbrot_c(chdata2, job2, width);
  fprintf (stdout, "mandelbrot errors = %s\n", errorMessageCL(error));
#if PRINT_MANDEL
  for (i=0; i < width; i++) {
    //if (chdata[i] != chdata2[i]) {
      fprintf(stdout, "mandelcl(%d) = %d vs %d\n", i, (int) chdata[i], (int) chdata2[i]);
    //}
  }
#endif

#if 0 
  float complex c = 10 + 5i;
  float complex d = 10 + 5i;
  cl_fract real = __real__ c;
  cl_fract imag = __imag__ c;
  //cl_fract e = ((real*real) + (imag*imag));
  c = c*c;
  cl_fract e = COMPLEX64ABSSQ(c);
  fprintf (stdout, "float e = %f, realc = %f, imagc = %f\n", e, __real__ c, __imag__ c);
  real = __real__ d;
  imag = __imag__ d;
  cl_fract iter_r = (real*real) - (imag*imag); \
  cl_fract iter_i = (imag*real) + (real*imag); \
  cl_fract f = (iter_r*iter_r)+(iter_i*iter_i);
  fprintf (stdout, "float f = %f, realc = %f, imagc = %f\n", f, iter_r, iter_i);
#endif

#if 1
  //modulo test
  error = 0;
  fprintf (stdout, "========= MODULO PRECISION TEST =========\n");
  error = 0;
  error += init_modulo();
  cl_int num = -30;
  cl_float amount = 0.123145;
  cl_float mod = 0;
  error += modulo(&num, &mod, &amount);
  fprintf (stdout, "CL exceution = %f\n", mod);
  cl_float res = (num - 50) / (amount * 2.0);
  res = (res * res) * res;
  fprintf (stdout, "C  execution = %f\n", res);
#endif

  return error;
}
