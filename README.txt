04/05/2023
The code in this repository is a port of Scilab code that I wrote last
year to implement an NLMS adaptive noise canceller.  The first step in
this effort is to "un-Scilab" the code.  That is, inner products have been
replaced with loops (which are actually performing inner products the
long way.  Soon, I will have a C++ class that implements this noise
canceller.  For now, I want to experiment with the Scilab code to flush
out any anomolies.

04/072023
We have compiling C++ code!

