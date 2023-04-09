//*************************************************************************
// File name: noisyCosine.cc
//*************************************************************************

//*************************************************************************
// This program tests the noise canceller by driving it with a floating
// point representation of a cosine wave.  Noise can be injected into
// the waveform so that the effectiveness of the noise canceller can be
// observed.
//
// To run this program type,
// 
//     ./noisyCosine -a amplitude -f frequency -r sampleRate
//                     -d duration -v noiseVariance ,
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
#include "NlmsNoiseCanceller.h"

// This structure is used to consolidate user parameters.
struct MyParameters
{
  float *amplitudePtr;
  float *frequencyPtr;
  float *sampleRatePtr;
  float *durationPtr;
  float *noiseVariancePtr;
  int *filterOrderPtr;
  int *delayPtr;
  float *betaPtr;
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
    opt = getopt(argc,argv,"a:f:r:t:v:o:d:b:h");

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

      case 't':
      {
        *parameters.durationPtr = atof(optarg);
        break;
      } // case

      case 'v':
      {
        *parameters.noiseVariancePtr = atof(optarg);
        break;
      } // case

      case 'o':
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
        fprintf(stderr,"./noisyCosine -a amplitude -f frequency -r sampleRate"
                " -t duration -v noiseVariance"
                " -o filterOrder -d delay -b beta\n");

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
  FILE *fd1;
  FILE *fd2;
  FILE *fd3;
  FILE *fd4;
  bool exitProgram;
  float noise;
  float iValue, qValue;
  float processedSample;
  float amplitude;
  float frequency;
  float sampleRate;
  float duration;
  float noiseVariance;
  int filterOrder;
  int delay;
  float beta;
  int numberOfSamples;
  Nco *myNcoPtr;
  NlmsNoiseCanceller *cancellerPtr;
  struct MyParameters parameters;

  // Set up for parameter transmission.
  parameters.amplitudePtr = &amplitude;
  parameters.frequencyPtr = &frequency;
  parameters.sampleRatePtr = &sampleRate;
  parameters.durationPtr = &duration;
  parameters.noiseVariancePtr = &noiseVariance;
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

  // We derive this.
  numberOfSamples = (int)(sampleRate * duration);

  // Instantiate an NCO.
  myNcoPtr = new Nco(sampleRate,frequency);

  // Instantiate a noise canceller.
  cancellerPtr = new NlmsNoiseCanceller(filterOrder,delay,beta);

  fd1 = fopen("original.dat","w");
  fd2 = fopen("noise.dat","w");
  fd3 = fopen("tainted.dat","w");
  fd4 = fopen("processed.dat","w");

  for (i = 0; i < numberOfSamples; i++)
  {
    // Get the next sample pair.
    myNcoPtr->run(&iValue,&qValue);

    // Get noise sample.
    noise = gauss(noiseVariance);

    // Write the untainted sample to the file..
    fwrite(&iValue,sizeof(float),1,fd1);

    // Write the noise sample to the file..
    fwrite(&noise,sizeof(float),1,fd2);

    // Add noise to sine wave.
    iValue = iValue + noise;

    // Write the noisy sample to the file..
    fwrite(&iValue,sizeof(float),1,fd3);

    // Remove the noise from the sample.
   cancellerPtr->acceptData(&iValue,1,&processedSample);

    // Write the processed sample to the file..
    fwrite(&processedSample,sizeof(float),1,fd4);
  } // for

  // We're done with these files.
  fclose(fd1);
  fclose(fd2);
  fclose(fd3);
  fclose(fd4);

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Release resources.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  if (myNcoPtr != NULL)
  {
    delete myNcoPtr;
  } // if

  if (cancellerPtr != NULL)
  {
    delete cancellerPtr;
  } // if
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  return (0);

} // main
