//**************************************************************************
// file name: NlmsNoiseCanceller.h
//**************************************************************************
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// This class implements a signal processing block known as an adaptive
// noise canceller.  A normalized LMS (least mean square) algorithm is
// used for the coefficient update equation.
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#ifndef __NLMSNOISECANCELLER__
#define __NLMSNOISECANCELLER__

#include <stdint.h>

#include "FirFilter.h"

class NlmsNoiseCanceller
{
  //***************************** operations **************************

  public:

  NlmsNoiseCanceller(int filterLength,int referenceDelay,float beta);
  ~NlmsNoiseCanceller(void);

  void acceptData(int16_t *bufferPtr,uint32_t bufferLength);
  float filterData(float x);

  private:

  //*******************************************************************
  // Utility functions.
  //*******************************************************************
  // Abstract the implementation of the pipeline.
  void shiftSampleInPipeline(float x);

  // This performs linear convolution.
  float dotProduct(float a,float b,int n);

  //*******************************************************************
  // Attributes.
  //*******************************************************************
  // The number of taps in the filter.
  int filterLength;
 
  // The number of samples to delay the input data, x, so
  //  that the reference signal, d(n) = x(n - n0), can be formed.
  int referenceDelay;

  // The adaptive filtering update (normalized step-size) parameter.
  float beta;

  // Pointer to the storage for the filter coefficients.
  float *coefficientStoragePtr;

  // Pointer to the filter state (previous samples).
  float *filterStatePtr;

  // This filter is used as a delay line.
  FirFilter *delayLinePtr;
};

#endif // __NLMSNOISECANCELLER__
