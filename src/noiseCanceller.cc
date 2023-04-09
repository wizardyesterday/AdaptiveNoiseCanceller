//*************************************************************************
// File name: noiseCanceller.cc
//*************************************************************************

//*************************************************************************
// This program tests both the NLMS noise canceller.  A noisy signal is
// read from stdin, and the reduced-noise signal is written to stdout.
//
// To run this program type,
// 
//     ./noiseCanceller -f filterOrder -d delay -b beta < inputFileName
//                      > outputFileName,
//
// where,
//
//    filterOrder - The order of the adaptive filter used for noise reduction.
//    delay - The delay that is used to generate the reference signal.
//*************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "NlmsNoiseCanceller.h"

// This structure is used to consolidate user parameters.
struct MyParameters
{
  int *filterOrderPtr;
  int *delayPtr;
  float *betaPtr;
};

int16_t inputBuffer[16384];
int16_t outputBuffer[16384];

/*****************************************************************************

  Name: getUserArguments

  Purpose: The purpose of this function is to retrieve the user arguments
  that were passed to the program.  Any arguments that are specified are
  set to reasonable default values.

  Calling Sequence: exitProgram = getUserArguments(parameters)

  Inputs:

    parameters - A structure that contains pointers to the user parameters.

  Outputs:

    exitProgram - A flag that indicates whether or not the program should
    be exited.  A value of true indicates to exit the program, and a value
    of false indicates that the program should not be exited..

*****************************************************************************/
bool getUserArguments(int argc,char **argv,struct MyParameters parameters)
{
  bool exitProgram;
  bool done;
  int opt;

  // Default not to exit program.
  exitProgram = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Default parameters.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Default a 5th order filter.
  *parameters.filterOrderPtr = 5;

  // Default to a delay of 5 samples.
  *parameters.delayPtr = 5;

  // Default to a convergence rate of something reasonable.
  *parameters.betaPtr = 0.1;
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Set up for loop entry.
  done = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Retrieve the command line arguments.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  while (!done)
  {
    // Retrieve the next option.
    opt = getopt(argc,argv,"f:d:h");

    switch (opt)
    {
      case 'f':
      {
        *parameters.filterOrderPtr = atoi(optarg);
        break;
      } // case

      case 'd':
      {
        *parameters.delayPtr = atoi(optarg);
        break;
      } // case

      case 'b':
      {
        *parameters.betaPtr = atof(optarg);
        break;
      } // case

      case 'h':
      {
        // Display usage.
        fprintf(stderr,"./noiseCanceller -f filterOrder -d delay -b beta\n");

        // Indicate that program must be exited.
        exitProgram = true;
        break;
      } // case

      case -1:
      {
        // All options consumed, so bail out.
        done = true;
      } // case
    } // switch

  } // while
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  return (exitProgram);

} // getUserArguments

//*************************************************************************
// Mainline code.
//*************************************************************************
int main(int argc,char **argv)
{
  bool exitProgram;
  bool done;
  uint32_t count;
  int filterOrder;
  int delay;
  float beta;
  NlmsNoiseCanceller *cancellerPtr;
  struct MyParameters parameters;

  // Set up for parameter transmission.
  parameters.filterOrderPtr = &filterOrder;
  parameters.delayPtr = &delay;
  parameters.betaPtr = &beta;

  // Retrieve the system parameters.
  exitProgram = getUserArguments(argc,argv,parameters);

  if (exitProgram)
  {
    // Bail out.
    return (0);
  } // if

  // Instantiate a noise canceller.
  cancellerPtr = new NlmsNoiseCanceller(filterOrder,delay,beta);

  // Set up for oop entry
  done = false;

  while (!done)
  {
    // Read a block of input samples.
    count = fread(inputBuffer,sizeof(int16_t),4000,stdin);

    if (count == 0)
    {
      // We're done.
      done = true;
    } // if
    else
    {
      // Remove the noise from the signal.
     cancellerPtr->acceptData(inputBuffer,count,outputBuffer);

      // Output the filtered data.
      fwrite(outputBuffer,sizeof(int16_t),count,stdout);
    } // else
  } // while

  // Release resources.
  if (cancellerPtr != NULL)
  {
    delete cancellerPtr;
  } // if

  return (0);

} // main
