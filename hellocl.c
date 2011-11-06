/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - bl73@kent.ac.uk
 */

#include "sin.h"
#include "rot13.h"
#include <stdlib.h>
#include <stdio.h>

#define SIN 1
#define PRINT_SOME 1
#define PRINT_ALL 0

int main() {
  char plaintext[]="Hello, World!";
  char ciphertext[sizeof plaintext];
  
  cl_int error;
#if SIN
  int i;
#endif

  // CL initialisation
  error = initialisecl();

#if SIN
  // SIN wave

  // Create and initialize the input data
  cl_float *data;
  data = (cl_float*)malloc(sizeof(cl_float)*10240);
  for (i=0; i<10240; i++) {
    data[i] = i;
  }
  // init sin
  error = init_sin();
  // run sin kernel
  error += clsin(data);
  fprintf (stdout, "errors = %s\n", errorMessageCL(error));
#if PRINT_ALL
  // Print out the results
  for (i=0; i<10240; i++)
    printf("sin(%d) = %f\n", i, data[i]);
#elif PRINT_SOME
  for (i=0; i<10240; i++) {
    printf("sin(%d) = %f\n", i, data[i]);
    i = i + 100;
  }
#endif
  // free data
  free (data);
#endif

  // rot13 initialisation
  error += init_rot13();
  fprintf (stdout, "errors = %s\n", errorMessageCL(error));

  // run rot13 CL kernel
  error += rot13(plaintext, ciphertext);

  // Finally, output out happy message.
  fprintf (stdout, "ciphertext = %s, errors = %d\n", ciphertext, error);

  return error;
}
