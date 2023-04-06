//**********************************************************************
// File Name: C9_9.sci
//**********************************************************************

// Bring in all of the utility functions.
exec('utils.sci',-1);

//**********************************************************************
//
//  Name: nlms_noiseCanceller
//
//  The purpose of this function is to perform noise cancelling using
//  the normalized LMS algorithm.  The reference signal is formed by
//  delaying the input signal by a specified number of samples such
//  that the noise portion of the delayed input signal is uncorrelated
//  with the nondelayed input signal.  This way, the adaptive filter
//  of the noise canceller will prodide an estimate of the desired
//  signal as its output.
//
//  Note: This code has been modified from the original
//  nlms_noiseCanceller() function so that it can easily be converted
//  to C or C++.  That is clear since the inner products have been
//  replaced with loops.  Additionally, the C or C++ code will work in a
//  streaming mode of operation versus working with blocks of data.  This
//  means that the function will be invoked one sample at a time.  I believe
//  that, although this may not be the most efficient way of doing things,
//  the code will be easier to understand.
//
//  Calling Sequence: [W,dyat] = nlms_noiseCanceller(x,n0,Beta,nord,w0)
//
//  Inputs:
//
//    x - The input data to the adaptive filter.
//
//    n0 - The number of samples to delay the input data, x, so
//    that the reference signal, d(n) = x(n - n0), can be formed.
//
//    Beta - The adaptive filtering update (normalized step-size)
//    parameter.
//
//    nord - The number of filter coefficients.
//
//    w0 - An optional row vector that serves as the initial guess
//    for FIR filter coefficients.  If w0 is omitted, then w0 = 0 is
//    assumed.
//    
//  Outputs:
//
//    dhat - A vector of estimations of the desired signal, d(n).
//    Each entry is associated with iteration n of the adaptive
//    filter.
//
//**********************************************************************
function dhat = knlms_noiseCanceller(x,n0,Beta,nord,w0)

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Since a reference signal is not provided to this function,
  // it is constructed by delaying the input signal by n0
  // samples.  Advantge is taken of the fact that the noise
  // component of the signal is no longer correlated with the
  // input signal, x(n) delayed by n0 samples.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Create filter coefficients for the delay line.
  b = zeros(1,n0+1);
  b($) = 1;

  // Construct the reference signal, d(n) = x(n-n0).
  d = filterBlock(x,b,0);
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Construct the data matrix.
  X = convm(x,nord);

  // Retrieve the size of the data matrix.
  [M,N] = size(X);

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // argn(2) returns is the number of arguments passed to the 
  // function.
  // If 4 arguments were passed to the function, it is implied
  // that the last parameter was not passed.  In this
  // case, the initial condition for the filter coefficients
  // is set to a default value of all zeros. 
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  if argn(2) < 5
    w0 = zeros(1,N);
  end

  // Force a row vector without altering the values.
  w0 = w0(:).';

  // Grab the first pipeline state.
  pipeline = X(1,:);

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Compute the first output value.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
   // Initialize the running sum.
   dhat(1) = 0;

  // Compute dhat = w0 * pipeline'.
  for j = 1:N
    dhat(1) = dhat(1) + w0(j) * pipeline(j);
  end
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Compute the first iteration for the error.
  E = d(1) - dhat(1); 

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Constructthe normalizing denominator for the first
  // iteration.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Initialize the running sum.
  DEN = 0;

  // Compute DEN = pipeline * pipeline' + 0.0001.
  for j = 1:N
    DEN = DEN + pipeline(j) * pipeline(j) + 0.0001;
  end
  DEN = DEN + 0.0001;
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Perform the first iteration for the filter vector.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Compute W = w0 + (Beta / DEN) * E * conj(pipeline);
  for j = 1:N
    W(j) = w0(j) + (Beta / DEN) * E(1) * conj(pipeline(j));
  end
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  if M > 1
    for k = 2:M - nord + 1

      // Grab the next pipeline state.
      pipeline = X(k,:);

      //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
      // Compute the next output.
      //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
       // Initialize the running sum.
       dhat(k) = 0;

      // Compute dhat = W * pipeline'.
      for j = 1:N
        dhat(k) = dhat(k) + W(j) * pipeline(j);
      end
      //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 
      // Compute the next error.
      E = d(k) - dhat(k);

      //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
      // Update the normalizing denominator.
       //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
     // Initialize the running sum.
      DEN = 0;

      // Compute DEN = pipeline * pipeline' + 0.0001.
      for j = 1:N
        DEN = DEN + pipeline(j) * pipeline(j) + 0.0001;
      end
      DEN = DEN + 0.0001;
      //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

      //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
      // Update the filter vector.
      //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
      // Compute W = W + (Beta / DEN) * E * conj(pipeline);
      for j = 1:N
        W(j) = W(j) + (Beta / DEN) * E * conj(pipeline(j));
      end
      //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    end
  end

endfunction

//**********************************************************************
// Mainline code.
//**********************************************************************
// Set filter orders.
p5 = 5;
p10 = 10;
p15 = 15;
p20 = 20;

// Set delays.
n5 = 5;
n10 = 10;
n25 = 25;

Beta = 0.1;

// Load noisy PCM file.
//fd = mopen('f162425.raw')
fd = mopen('f1354.raw')
//x = mget(323584,'s',fd);
x = mget(10000,'s',fd);
mclose(fd);

dhat = knlms_noiseCanceller(x,n5,Beta,p5);
//dhat = knlms_noiseCanceller(x,n5,Beta,p10);
//dhat = knlms_noiseCanceller(x,n5,Beta,p15);
//dhat = knlms_noiseCanceller(x,n5,Beta,p20);

fd = mopen('kdhat1354.raw','w');
mput(dhat,'s',fd);
mclose(fd);


////_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// Plot results.
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/


