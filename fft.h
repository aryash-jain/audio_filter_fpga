#ifndef FFT_H
#define FFT_H

#include <complex>
#include <ap_fixed.h>

typedef ap_fixed<16,4> fixed_t;
typedef std::complex<fixed_t> cmpxData;

void fft_wrapper(fixed_t in[], cmpxData out[]);
void ifft_wrapper(cmpxData in[], fixed_t out[]);

#endif
