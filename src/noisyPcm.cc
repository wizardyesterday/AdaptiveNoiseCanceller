//*************************************************************************
// File name: noisyCosine.cc
//*************************************************************************

//*************************************************************************
// This program tests anadaptive noise cnceller when driven by PCM data
// that has noise injected into the samples.  Two  selectable output file
// types . Thll be written to sdtout.e first file will contain PCM samples
// that are perturbed by additive white Gaussian noise. The second file
// will contain noise-reduced PCM samples. The user selects the output
// file type via a command line argument.
//
// To run this program type,
// 
//     ./noisyCosine -t filetype -v noiseVariance > outputFileName,
//
// where,
//
//    diletype - Either noisy or noise-reduced.
//    noiseVariance - The variance of the noise source.
///*************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "NlmsNoiseCanceller.h"

// This structure is used to consolidate user parameters.
struct MyParameters
{
  int *fileTypePtr;
  float *noiseVariancePtr;
  int *filterLengthPtr;
  int *delayPtr;
  float *betaPtr;
};

// Globals.
int16_t inputBuffer[32768];
int16_t outputBuffer[32768];
float floatBuffer[32768];

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
  // Default to a file type of noisy.
  *parameters.fileTypePtr = 0; 

  // Default to a noise variance of 0.1;
  *parameters.noiseVariancePtr = 0.1;

  // Default to a LMS filter length 0f 5.
  *parameters.filterLengthPtr = 5;

  // Default to a LMS delay of 5.
  *parameters.delayPtr = 5;

  // Default to a LMS delay convergence factor of 0.1
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
    opt = getopt(argc,argv,"t:v:l:d:vh");

    switch (opt)
    {
      case 't':
      {
        *parameters.fileTypePtr = atoi(optarg);
        break;
      } // case

      case 'v':
      {
        *parameters.noiseVariancePtr = atof(optarg);
        break;
      } // case

      case 'l':
      {
        *parameters.filterLengthPtr = atoi(optarg);
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
        fprintf(stderr,"./noisyPcm -t fileType [0, noisy | 1, noise-reduced"
                " -v noiseVariance\n -l filterLength -d delay -b beta\n");

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

/*****************************************************************************

  Name: gauss

  Purpose: The purpose of this function is to generate a random number
  that is weighted by a Gaussian density function. 

  Calling Sequence: value = gauss(sigma)

  Inputs:

    sigma - The standard deviation of the random process.

  Outputs:

    value - The generated random number.

*****************************************************************************/
float gauss(float sigma)
{
  float x, b, r, value;

  // Get first random variable.
  x = (float)rand();

  // Scale the random variable.
  x = x / RAND_MAX;

  // Get second random variable.
  b = (float)rand();

  // Scale the random variable.
  b = b / RAND_MAX;

  // Generate the angle.
  b = 2.0 * b * M_PI;

  // Compute the magnitude.
  r = sqrt(2.0 * sigma * sigma * log(1.0 / (1.0 - x)));

  // Compute the real part of the random variable.
  value = r * cos(b);

  return (value);

} // gauss

//*************************************************************************
// Mainline code.
//*************************************************************************
int main(int argc,char **argv)
{
  int i;
  bool exitProgram;
  int fileType;
  float noiseVariance;
  int filterLength;
  int delay;
  float beta;
  float noise;
  float sigma;
  uint32_t count;
  bool done;
  NlmsNoiseCanceller *myCancellerPtr;
  struct MyParameters parameters;

  // Set up for parameter transmission.
  parameters.fileTypePtr = &fileType;
  parameters.noiseVariancePtr = &noiseVariance;
  parameters.filterLengthPtr = &filterLength;
  parameters.delayPtr = &delay;
  parameters.betaPtr = &beta;

  // Retrieve the system parameters.
  exitProgram = getUserArguments(argc,argv,parameters);

  if (exitProgram)
  {
    // Bail out.
    return (0);
  } // if

  // Convert to standard deviation.
  sigma = sqrt(noiseVariance);

  // Instantiate an adaptive noise canceller
  myCancellerPtr = new NlmsNoiseCanceller(filterLength,delay,beta);

  // Set up for loop entry.
  done = false;

  while (!done)
  {
    // Read a block of PCM samples.
    count = fread(inputBuffer,sizeof(int16_t),1024,stdin);

    if (count == 0)
    {
      // We're done.
      done = true;
    } // if
    else
    {
      for (i = 0; i < count; i++)
      {
        // Scaled to a maximum magnitude of unity.
        floatBuffer[i] = (float)inputBuffer[i] / 32768;

        // Generate a noise sample.
        noise = gauss(sigma);

        // Add noise.
        floatBuffer[i] += noise;

        // Convert to PCM ample.
        floatBuffer[i] *= 32000;

      } // for

      switch (fileType)
      {
        case 0:
        {
          // The noisy PCM data will be used.
        } // case

        case 1:
        {
          // The noise-reduced data wil be used.
          myCancellerPtr->acceptData(floatBuffer,count,floatBuffer);
        } // case
      } // switch

      for (i = 0; i < count; i++)
      {
        // Convert to PCM samples.
        outputBuffer[i] = (int16_t)(floatBuffer[i] * 32768);
      } // for
    } // else

    fwrite(outputBuffer,sizeof(int16_t),count,stdout);
  } // while

  // Release resources.
  if (myCancellerPtr != NULL)
  {
    delete myCancellerPtr;
  } // if

  return (0);

} // main
