//*************************************************************************
// File name: noisyCosine.cc
//*************************************************************************

//*************************************************************************
// This program tests both the numerically controlled oscillator (NCO).
// The NCO data is written to an output file with the format described
// below.  Note that only the in-phase component of the NCO output is
// used, therefore the output file represents a cosine waveform.  The
// output waveform can contain a user-specified amount of noise.
//
// To run this program type,
// 
//     ./noisyCosine -a amplitude -f frequency -r sampleRate
//                     -d duration -v noiseVariance > outputFileName,
//
// where,
//
//    amplitude - The ampoitude between 0 and 1 inclusive.
//    frequency - frequency in Hz.
//    sampleRate - The sample rate in samples/second.
//    duration - The duration in seconds.
//    noiseVariance - The variance of the noise source.
///*************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "Nco.h"

// This structure is used to consolidate user parameters.
struct MyParameters
{
  float *amplitudePtr;
  float *frequencyPtr;
  float *sampleRatePtr;
  float *durationPtr;
  float *noiseVariancePtr;
};

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
  // Default 1/2 scale.
  *parameters.amplitudePtr = 0.5;

  // Default to 200Hz.
  *parameters.frequencyPtr = 200;

  // Default to 24000 S/s.
  *parameters.sampleRatePtr = 24000;

  // Default for a 1 second signal.
  *parameters.durationPtr = 1;

  // Default to a noise variance of 0.1;
  *parameters.noiseVariancePtr = 0.1;
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Set up for loop entry.
  done = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Retrieve the command line arguments.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  while (!done)
  {
    // Retrieve the next option.
    opt = getopt(argc,argv,"a:f:r:d:v:h");

    switch (opt)
    {
      case 'a':
      {
        *parameters.amplitudePtr = atof(optarg);
        *parameters.amplitudePtr = fabs(*parameters.amplitudePtr);
        break;
      } // case

      case 'f':
      {
        *parameters.frequencyPtr = atof(optarg);
        break;
      } // case

      case 'r':
      {
        *parameters.sampleRatePtr = atof(optarg);
        break;
      } // case

      case 'd':
      {
        *parameters.durationPtr = atof(optarg);
        break;
      } // case

      case 'v':
      {
        *parameters.noiseVariancePtr = atof(optarg);
        break;
      } // case

      case 'h':
      {
        // Display usage.
        fprintf(stderr,"./noisyCosine -a amplitude -f frequency -r sampleRate"
                " -d duration -v noiseVariance\n");

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
  float iValue, qValue;
  int16_t cosineValue;
  float amplitude;
  float frequency;
  float sampleRate;
  float duration;
  float noiseVariance;
  int numberOfSamples;
  float noise;
  Nco *myNcoPtr;
  struct MyParameters parameters;

  // Set up for parameter transmission.
  parameters.amplitudePtr = &amplitude;
  parameters.frequencyPtr = &frequency;
  parameters.sampleRatePtr = &sampleRate;
  parameters.durationPtr = &duration;
  parameters.noiseVariancePtr = &noiseVariance;

  // Retrieve the system parameters.
  exitProgram = getUserArguments(argc,argv,parameters);

  if (exitProgram)
  {
    // Bail out.
    return (0);
  } // if

  // We derive this.
  numberOfSamples = (int)(sampleRate * duration);

  // Instantiate an NCO.
  myNcoPtr = new Nco(sampleRate,frequency);

  for (i = 0; i < numberOfSamples; i++)
  {
    // Get the next sample pair.
    myNcoPtr->run(&iValue,&qValue);

    // Get noise sample.
    noise = gauss(noiseVariance);

    // Add noise to sine wave.
    iValue = iValue + noise;

    // Convert to integer and scale.
    cosineValue = (int16_t)(iValue * amplitude * 32767);

    // Write the samples to stdout
    fwrite(&cosineValue,sizeof(int16_t),1,stdout);
  } // for

  // Release resources.
  if (myNcoPtr != NULL)
  {
    delete myNcoPtr;
  } // if

  return (0);

} // main
