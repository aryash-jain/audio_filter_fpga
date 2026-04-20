#ifndef FFT_H
#define FFT_H

#include <complex>
#include <hls_fft.h>
#include "PitchTrans.h"

typedef ap_fixed<24, 1> fft_data_t;
typedef std::complex<fft_data_t> fft_cmpx_t;

typedef std::complex<fixed_t> cmpxData;

void fft_wrapper(fixed_t in[FRAME_SIZE], cmpxData out[FRAME_SIZE]);
void ifft_wrapper(cmpxData in[FRAME_SIZE], fixed_t out[FRAME_SIZE]);

#endif
