/* Simple library meant to be called from occam that serves as proof 
 * of concept for occam -> C/opencl interopability
 *
 * Copyright 2011 - Brendan Le Foll - brendan@fridu.net
 * Copyright 2011 - University of Kent
 */

#include <argp.h>

#include "hellocl_test.h"

const char *argp_program_version = "hellocl 1.0";
const char *argp_program_bug_address = "<brendan@fridu.net>";
static char doc[] = "hellocl -- Test program for occam-pi opencl library";
/* A description of the arguments we accept. */
static char args_doc[] = "ARG1 ARG2";
     
/* The options we understand. */
static struct argp_option options[] = {
  {"nocharext",'c', 0, 0, "Stop tests that require cl_khr_byte_addressable_store", 0}, 
  {"mandeliterations", 'i', 0, 0, "Specify the amount of mandelbrot iterations to be done", 0},
  {"verbose",  'v', 0, 0, "Produce verbose output", 0},
  {"quiet",    'q', 0, 0, "Don't produce any output", 0},
  {0, 0, 0, 0, 0, 0}
};
     
struct arguments
{
  char *args[2];                /* arg1 & arg2 */
  int silent, verbose, nocharext, mandeliterations;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
      know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;
     
  switch (key)
  {
    case 'q': case 's':
      arguments->silent = 1;
      break;
    case 'v':
      arguments->verbose = 1;
      break;
    case 'c':
      arguments->nocharext = 1;
      break;
    case 'i':
      arguments->mandeliterations = 8;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

int main (int argc, char *argv[])
{
  cl_int error;
  struct arguments arguments;
     
  /* Default values. */
  arguments.silent = 0;
  arguments.verbose = 0;
  arguments.nocharext = 0;
  arguments.mandeliterations = 25;

  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  // CL initialisation
  error = initialisecl();
  if(error != CL_SUCCESS) {
    fprintf (stdout, "initialisecl() returned %s", errorMessageCL(error));
    return 1;
  }

  // Some general information
  fprintf (stdout, "========= NEW CL DEVICE =========\n");
  printDeviceName();
  printDevExt();

  // MODULO PRECISION
  moduloTest();

  // SIN
  sinTest();

  // check for --nocharext
  if (!arguments.nocharext) {
    // Check device supports extensions we need for rot13 & mandelbrot
    char *khr_byte = "cl_khr_byte_addressable_store";
    if (getCorrectDevice(khr_byte)) {
      return 2;
    }

    // ROT13
    rot13Test();

    char *khr_fp64 = "cl_khr_fp64";
    if (getCorrectDevice(khr_fp64)) {
      return 3;
    }

    // MODULO PRECISION
    moduloTest();

    // MANDELBROT
    mandelbrotTest(arguments.verbose, arguments.mandeliterations);
  }

  return error;
}
