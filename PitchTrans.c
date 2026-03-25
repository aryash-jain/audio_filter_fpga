#include "PitchTrans.h"
#include "fft.h"
#include <hls_math.h>

void pitch_shift(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], fixed_t tones) {
    const int N = FRAME_SIZE;
    const int H = HOP_SIZE;
    const fixed_t PI = 3.14159265358979;

    fixed_t ratio = hls::pow((fixed_t)2.0, tones / (fixed_t)12.0);

    fixed_t hann[FRAME_SIZE];
    fixed_t frame[FRAME_SIZE];
    fixed_t outFrame[FRAME_SIZE];

    cmpxData spec[FRAME_SIZE];
    cmpxData shiftSpec[FRAME_SIZE];

    // Hann window
    for (int n = 0; n < N; n++) {
        #pragma HLS PIPELINE II=1
        fixed_t wave = hls::cos((fixed_t)(2.0 * PI * n / (N - 1)));
        hann[n] = (fixed_t)0.5 * ((fixed_t)1.0 - wave);
    }

    // Initialize final output
    for (int j = 0; j < SAMPLE_SIZE; j++) {
        #pragma HLS PIPELINE II=1
        output[j] = 0;
    }

    // Frame processing
    for (int k = 0; k <= SAMPLE_SIZE - N; k += H) {

        for (int j = 0; j < N; j++) {
            #pragma HLS PIPELINE II=1
            frame[j] = (fixed_t)input[k + j] * hann[j];
        }

        fft_wrapper(frame, spec);

        for (int j = 0; j < N; j++) {
            #pragma HLS PIPELINE II=1
            shiftSpec[j] = cmpxData(0, 0);
        }

        for (int m = 0; m < N / 2; m++) {
            #pragma HLS PIPELINE II=1

            int newK = (int)(m * ratio + (fixed_t)0.5);

            if (newK > 0 && newK < N / 2) {
                shiftSpec[newK] += spec[m];
                shiftSpec[N - newK] += spec[N - m];
            }
        }

        ifft_wrapper(shiftSpec, outFrame);

        for (int j = 0; j < N; j++) {
            #pragma HLS PIPELINE II=1

            accum_t val = (accum_t)(outFrame[j] * hann[j]);
            accum_t sum = (accum_t)output[k + j] + val;

            if (sum > 32767) sum = 32767;
            if (sum < -32768) sum = -32768;

            output[k + j] = (short)sum;
        }
    }
}