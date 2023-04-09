#!/bin/sh
#*****************************************************************************
# File name: buildCosine.sh
#*****************************************************************************
# This build script creates the cosine app.
# Chris G. 07/23/2021
#*****************************************************************************
g++ -I include -g -O0 -o noisyCosine src/noisyCosine.cc src/Nco.cc src/PhaseAccumulator.cc src/FirFilter.cc src/NlmsNoiseCanceller.cc

g++ -I include -g -O0 -o noiseCanceller src/noiseCanceller.cc src/FirFilter.cc src/NlmsNoiseCanceller.cc

