#ifndef FFT_H
#define FFT_H

#include <complex>
#include "PitchTrans.h"

typedef std::complex<fixed_t> cmpxData;

void fft_wrapper(fixed_t in[FRAME_SIZE], cmpxData out[FRAME_SIZE]);
void ifft_wrapper(cmpxData in[FRAME_SIZE], fixed_t out[FRAME_SIZE]);

#endif