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

  // Allocate delay line storage
  delayLineCoefficientsPtr = new float[referenceDelay];

  // Set delay line coefficient.
  delayLineCoefficientsPtr[referenceDelay - 1] = 1;

  // Instantiate delay line.
  delayLinePtr = new FirFilter(referenceDelay,delayLineCoefficientsPtr);

  // We don't need this anymore.
  delete[] delayLinePtr;

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

  Name: shiftSampleInPipeline

  Purpose: The purpose of this function is to shift the next sample into
  the filter state memory (the pipeline).  For now, a linear buffer will
  be used.  This was chosen because the pipeline is used in the update
  equation for the the filter coefficients.

  Calling Sequence: shiftSampleInPipeline(x)

  Inputs:

    x - The sample to shift into the pipeline.

  Outputs:

    None.

*****************************************************************************/
void NlmsNoiseCanceller::shiftSampleInPipeline(float x)
{
  int i;

  // Shift the existing samples.
  for (i = 1; i < filterLength; i++)
  {
    // Make room for the new sample.
    filterStatePtr[i] = filterStatePtr[i-1];
  } // for

  // Place the sample into the pipeline.
  filterStatePtr[0] = x;

  return;

} // shiftSampleInPipeline

/*****************************************************************************

  Name: dotProduct

  Purpose: The purpose of this function is to compute the dot product
  between two vectors

  Calling Sequence: dotProduct(a,b,n)

  Inputs:

    a - The first vector for which a dot product is to be computed.

    b - The second vector for which a dot product is to be computed.

  Outputs:

    c - The dot product of vectors, a and b.

*****************************************************************************/
float NlmsNoiseCanceller::dotProduct(float a[],float b[],int n)
{
  float result;
  int i;

  // Start out with a zero sum.
  result = 0;

  for (i = 0; i < n; i++)
  {
    result = result + (a[i] * b[i]);
  } // for

  return (result);

} // dotProduct

/*****************************************************************************

  Name: filterData

  Purpose: The purpose of this function is to filter one sample of data
  for the purpose of removing noise from a signal.


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

  // Compute reference sample.
  d = delayLinePtr->filterData(x);

  // Compute noise-reduced sample.
  dHat = dotProduct(w,filterStatePtr,filterLength);

  // Compute the error.
  e = d - dHat;

  // Compute the normalizing denominator.
  den = dotProduct(filterStatePtr,filterStatePtr,filterLength);

  // Update the filter coefficients.
  for (i = 0; i < filterLength; i++)
  {
    w[i] = w[i] + ((beta/den) * e * filterStatePtr[i]);
  } // for
 
  return (dHat);

} // filterData

