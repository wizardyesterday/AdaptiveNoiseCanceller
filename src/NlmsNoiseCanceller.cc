//************************************************************************
// file name: LmsNoiseCanceller.cc
//************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "LmsNoiseCanceller.h"

using namespace std;

/*****************************************************************************

  Name: LmsNoiseCanceller

  Purpose: The purpose of this function is to serve as the constructor for
  an instance of an LmsNoiseCanceller.

  Calling Sequence: LmsNoiseCanceller(filterLength,coefficientsPtr)

  Inputs:

    filterLength - The number of taps for the filter.

    coefficientPtr - A pointer to the filter coefficients.

  Outputs:

    None.

*****************************************************************************/
LmsNoiseCanceller::LmsNoiseCanceller(int filterLength,
                     float *coefficientsPtr)
{
  int i;

  // Save for later use.
  this->filterLength = filterLength;

  // Allocate storage for the coefficients.
  coefficientStoragePtr = new float[filterLength];

  // Save the coefficients.
  for (i = 0; i < filterLength; i++)
  {
    coefficientStoragePtr[i] = coefficientsPtr[i];
  } // for

  // Allocate storage for the filter state.
  filterStatePtr = new float[filterLength];

  // Set the filter state to an initial value.
  resetFilterState();

  return;

} // LmsNoiseCanceller

/*****************************************************************************

  Name: ~LmsNoiseCanceller

  Purpose: The purpose of this function is to serve as the destructor for
  an instance of an LmsNoiseCanceller.

  Calling Sequence: ~LmsNoiseCanceller()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
LmsNoiseCanceller::~LmsNoiseCanceller(void)
{

  // Release resources.
  delete[] coefficientStoragePtr;
  delete[] filterStatePtr;

  return;

} // ~LmsNoiseCanceller

/*****************************************************************************

  Name: shiftSampleInPipeline

  Purpose: The purpose of this function is to shift the next sample into
  the filter state memory (the pipeline).  For now, a linear buffer will
  be used.  This was chosen because the pipeline is used in the update
  equation for the the filter coefficients.

  Calling Sequence: shiftSampleInPipeline(float x)

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
void LmsNoiseCanceller::shiftSampleInPipeline(float x)
{
  int i;

  // Shift the existing samples.
  for (i = 2; i < filterLength; i++)
  {
    filterStatePtr[i] = filterStatePtr[i-1];
  } // for

  // Place the sample into the pipeline.
  filterStatePtr[0] = x;

  return;

} // shiftSampleInPipeline

/*****************************************************************************

  Name: filterData

  Purpose: The purpose of this function is to filter one sample of data.
  It uses a circular buffer to avoid the copying of data when the filter
  state memory is updated.

  Calling Sequence: y = filterData(x)

  Inputs:

    x - The data sample to filter.

  Outputs:

    y - The output value of the filter.

*****************************************************************************/
float LmsNoiseCanceller::filterData(float x)
{
  float *h, y;
  int k, xIndex;

  // Reference the first filter coefficient.
  h = coefficientStoragePtr;

  // Store sample value.
  filterStatePtr[ringBufferIndex] = x;

  // Set current position of index to deal with convolution sum.
  xIndex = ringBufferIndex;

  // Clear the accumulator.
  y = 0;
  
  for (k = 0; k < filterLength; k++)
  {
    // Perform multiply-accumulate operation.
    y = y + (h[k] * filterStatePtr[xIndex]);

    // Decrement the index in a modulo fashion.
    xIndex--;
    if (xIndex < 0)
    {
      // Wrap the index.
      xIndex = filterLength - 1;
    } // if
  } // for

  // Increment the index in a modulo fashion.
  ringBufferIndex++;
  if (ringBufferIndex == filterLength)
  {
    // Wrap the index.
    ringBufferIndex = 0;
  } // if
 
  return (y);

} // filterData

