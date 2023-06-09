//************************************************************************
// file name: NlmsNoiseCanceller.cc
//************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "NlmsNoiseCanceller.h"

using namespace std;

/*****************************************************************************

  Name: NlmsNoiseCanceller

  Purpose: The purpose of this function is to serve as the constructor for
  an instance of an NlmsNoiseCanceller.

  Calling Sequence: NlmsNoiseCanceller(filterLength,coefficientsPtr)

  Inputs:

    filterLength - The number of taps for the filter.

    coefficientPtr - A pointer to the filter coefficients.

  Outputs:

    None.

*****************************************************************************/
NlmsNoiseCanceller::NlmsNoiseCanceller(int filterLength,
                                       int referenceDelay,
                                       float beta)
{
  int i;
  float *delayLineCoefficientsPtr;

  // Save for later use.
  this->filterLength = filterLength;

  // Allocate storage for the coefficients.
  coefficientStoragePtr = new float[filterLength];

  // Start with zero-valued coefficients.
  for (i = 0; i < filterLength; i++)
  {
    coefficientStoragePtr[i] = 0;
  } // for

  // Allocate storage for the filter state.
  filterStatePtr = new float[filterLength];

  // Save this for display purposes.
  this->referenceDelay = referenceDelay;

  // Allocate delay line storage.
  delayLineCoefficientsPtr = new float[referenceDelay + 1];

  // Set delay line coefficient.
  delayLineCoefficientsPtr[referenceDelay] = 1;

  // Instantiate delay line.
  delayLinePtr = new FirFilter(referenceDelay+1,delayLineCoefficientsPtr);

  // We're done with this.
  delete[] delayLineCoefficientsPtr;

  // We'll use this for the update equation.
  this->beta = beta;

  return;

} // NlmsNoiseCanceller

/*****************************************************************************

  Name: ~NlmsNoiseCanceller

  Purpose: The purpose of this function is to serve as the destructor for
  an instance of an NlmsNoiseCanceller.

  Calling Sequence: ~NlmsNoiseCanceller()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
NlmsNoiseCanceller::~NlmsNoiseCanceller(void)
{

  // Release resources.
  delete[] coefficientStoragePtr;
  delete[] filterStatePtr;
  delete delayLinePtr;

  return;

} // ~NlmsNoiseCanceller

/*****************************************************************************

  Name: acceptData

  Purpose: The purpose of this function is to present input samples to
  be filtered and produce output samples to the calling function.

  Calling Sequence: acceptData(bufferPtr,bufferLength,outputBufferPtr)

  Inputs:

    bufferPtr - A pointer to storage that provides the input samples.

    bufferLength - The nmber of samples referenced by bufferPtr.  This
    will also be the number of samples stored into memory referenced
    by outputBufferPtr.

    outputBufferPtr - A pointer to storage for the processed samples.

  Outputs:

    None.

*****************************************************************************/
void NlmsNoiseCanceller::acceptData(int16_t *bufferPtr,
                                    uint32_t bufferLength,
                                    int16_t *outputBufferPtr)
{
  int i;

  // Filter the block of data provided by the caller.
  for (i = 0; i < bufferLength; i++)
  {
    outputBufferPtr[i] = (int16_t)filterData((float)bufferPtr[i]);
  } // for

  return;

} // acceptData

/*****************************************************************************

  Name: acceptData

  Purpose: The purpose of this function is to present input samples to
  be filtered and produce output samples to the calling function.

  Calling Sequence: acceptData(bufferPtr,bufferLength,outputBufferPtr)

  Inputs:

    bufferPtr - A pointer to storage that provides the input samples.

    bufferLength - The nmber of samples referenced by bufferPtr.  This
    will also be the number of samples stored into memory referenced
    by outputBufferPtr.

    outputBufferPtr - A pointer to storage for the processed samples.

  Outputs:

    None.

*****************************************************************************/
void NlmsNoiseCanceller::acceptData(float *bufferPtr,
                                    uint32_t bufferLength,
                                    float *outputBufferPtr)
{
  int i;

  // Filter the block of data provided by the caller.
  for (i = 0; i < bufferLength; i++)
  {
    outputBufferPtr[i] = filterData(bufferPtr[i]);
  } // for

  return;

} // acceptData

/*****************************************************************************

  Name: shiftSampleIntoPipeline

  Purpose: The purpose of this function is to shift the next sample into
  the filter state memory (the pipeline).  For now, a linear buffer will
  be used.  This was chosen because the pipeline is used in the update
  equation for the the filter coefficients.  The structure of the
  pipeline is,

  {x(n) x(n-1) x(n-2)...,x(n - N + 1)}.

  Calling Sequence: shiftSampleIntoPipeline(x)

  Inputs:

    x - The sample to shift into the pipeline.

  Outputs:

    None.

*****************************************************************************/
void NlmsNoiseCanceller::shiftSampleIntoPipeline(float x)
{
  int i;

  // Shift the existing samples.
  for (i = filterLength-1; i > 0; i--)
  {
    // Make room for the new sample.
    filterStatePtr[i] = filterStatePtr[i-1];
  } // for

  // Place the sample into the pipeline.
  filterStatePtr[0] = x;

  return;

} // shiftSampleIntoPipeline

/*****************************************************************************

  Name: dotProduct

  Purpose: The purpose of this function is to compute the dot product
  between two vectors

  Calling Sequence: dotProduct(aPtr,bPtr,n)

  Inputs:

    a - A pointer to the first vector for which a dot product is to be
    computed.

    b - A pointer to he second vector for which a dot product is to be
    computed.

  Outputs:

    c - The dot product of the two input vectors.

*****************************************************************************/
float NlmsNoiseCanceller::dotProduct(float *aPtr,float *bPtr,int n)
{
  float result;
  int i;

  // Start out with a zero sum.
  result = 0;

  for (i = 0; i < n; i++)
  {
    result = result + (aPtr[i] * bPtr[i]);
  } // for

  return (result);

} // dotProduct

/*****************************************************************************

  Name: filterData

  Purpose: The purpose of this function is to filter one sample of data
  for the purpose of removing noise from a signal.  Here's how it works.
  A reference signal is formed by delaying the input signal by a specified
  number of samples.  The idea here is that the noise portion of the
  delayed signal is uncorrelated with the nondelayed input signal.  This
  way, the adaptive filter of the noise canceller will provide an
  estimate of the desired signal as its output.

  Calling Sequence: dHat = filterData(x)

  Inputs:

    x - The data sample to filter.

  Outputs:

    dHat - The output value of the filter.  This is an estimate of a
    noise-reduced sample.

*****************************************************************************/
float NlmsNoiseCanceller::filterData(float x)
{
  int i;
  float dHat;
  float *w;
  float d;
  float den;
  float e;

  // Reference filter coefficients.
  w = coefficientStoragePtr;

  // Place the sample into the state memory.
  shiftSampleIntoPipeline(x);

  // Compute reference sample.
  d = delayLinePtr->filterData(x);

  // Compute noise-reduced sample.
  dHat = dotProduct(w,filterStatePtr,filterLength);

  // Compute the error.
  e = d - dHat;

  // Compute the normalizing denominator.
  den = dotProduct(filterStatePtr,filterStatePtr,filterLength);
  den += 0.0001;

  // Update the filter coefficients.
  for (i = 0; i < filterLength; i++)
  {
    w[i] = w[i] + ((beta / den) * e * filterStatePtr[i]);
  } // for
 
  return (dHat);

} // filterData

