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
  {"nocharext",        'c', 0, 0, "Stop tests that require cl_khr_byte_addressable_store", 0},
  {"mandeliterations", 'i', 0, 0, "Specify the amount of mandelbrot iterations to be done", 0},
  {"verbose",          'v', 0, 0, "Produce verbose output", 0},
  {"quiet",            'q', 0, 0, "Don't produce any output", 0},
  {"deviceinfo",       'd', 0, 0, "Print detailed device info and exit"},
  {0, 0, 0, 0, 0, 0}
};

struct arguments
{
  char *args[2];                /* arg1 & arg2 */
  int silent, verbose, nocharext, mandeliterations, info;
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
    case 'd':
      arguments->info = 1;
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
  arguments.info = 0;

  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  // CL initialisation
  error = initialisecl();
  if(error != CL_SUCCESS) {
    fprintf (stdout, "initialisecl() returned %s\n", errorMessageCL(error));
    return 1;
  }

  if (arguments.info) {
    // Print all the info about the CL device
    printPlatformInfo();
    printDevInfo();
    return CL_SUCCESS;
  }

  // Some general information
  fprintf (stdout, "========= CL DEVICE =========\n");
  printDeviceName();
  printDevExt();

  // SIN
  sinTest();

  // OCCOIDS
  occoidsTest();

  // MANDELBROT VIS TEST
  mandelbrotVisTest();

  // check for --nocharext
  if (!arguments.nocharext) {
    // Check device supports extensions we need for rot13 & mandelbrot
    if (getCorrectDevice("cl_khr_byte_addressable_store")) {
      fprintf (stdout, "No devices supporting cl_khr_byte_addressable_store found - bypassing rot13, mandelbrot and modulot tests.\n");
      return 2;
    }

    // ROT13
    rot13Test();

    // MANDELBROT
    mandelbrotTest(arguments.verbose, arguments.mandeliterations);

    if (getCorrectDevice("cl_khr_fp64")) {
      fprintf (stdout, "No devices supporting cl_khr_fp64 found - bypassing modulo tests.\n");
      return 3;
    }

    // MODULO PRECISION
    moduloTest();
  }

  // cleaup cl
  destroycl();

  return error;
}
