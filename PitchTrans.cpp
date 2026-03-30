#include "PitchTrans.h"
#include "fft.h"
#include <hls_math.h>

void pitch_shift(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], fixed_t tones) {
    const int N = FRAME_SIZE;
    const int H = HOP_SIZE;

    /* --- compute pitch ratio in float, cast back --- */
    float exp_f = (float)tones / 12.0f;
    fixed_t ratio = (fixed_t)hls::powf(2.0f, exp_f);

    /* --- Hann window --- */
    fixed_t hann[FRAME_SIZE];
    for (int n = 0; n < N; n++) {
#pragma HLS PIPELINE II=1
        float angle = 2.0f * 3.14159265f * (float)n / (float)(N - 1);
        hann[n] = (fixed_t)(0.5f * (1.0f - hls::cosf(angle)));
    }

    /* --- normalize input: short [-32768,32767] → fixed_t [-1.0, 1.0) --- */
    fixed_t norm_in[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++) {
#pragma HLS PIPELINE II=1
        norm_in[i] = (fixed_t)((float)input[i] / 32768.0f);
    }

    /* --- clear accumulator --- */
    accum_t acc[SAMPLE_SIZE];
    for (int j = 0; j < SAMPLE_SIZE; j++) {
#pragma HLS PIPELINE II=1
        acc[j] = 0;
    }

    /* --- overlap-add frame processing --- */
    fixed_t frame[FRAME_SIZE];
    fixed_t outFrame[FRAME_SIZE];
    cmpxData spec[FRAME_SIZE];
    cmpxData shiftSpec[FRAME_SIZE];

    for (int k = 0; k <= SAMPLE_SIZE - N; k += H) {

        /* window the frame */
        for (int j = 0; j < N; j++) {
#pragma HLS PIPELINE II=1
            frame[j] = norm_in[k + j] * hann[j];
        }

        fft_wrapper(frame, spec);

        /* clear shifted spectrum */
        for (int j = 0; j < N; j++) {
#pragma HLS PIPELINE II=1
            shiftSpec[j] = cmpxData(0, 0);
        }

        /* frequency-domain pitch shift */
        for (int m = 0; m < N / 2; m++) {
#pragma HLS PIPELINE II=1
            int newK = (int)(m * ratio + (fixed_t)0.5);
            if (newK > 0 && newK < N / 2) {
                shiftSpec[newK]     += spec[m];
                shiftSpec[N - newK] += spec[N - m];
            }
        }

        ifft_wrapper(shiftSpec, outFrame);

        /* overlap-add with window */
        for (int j = 0; j < N; j++) {
#pragma HLS PIPELINE II=1
            acc[k + j] += (accum_t)(outFrame[j] * hann[j]);
        }
    }

    /* --- denormalize: fixed_t [-1.0, 1.0) → short, with clipping --- */
    for (int i = 0; i < SAMPLE_SIZE; i++) {
#pragma HLS PIPELINE II=1
        accum_t val = acc[i] * (accum_t)32768.0;
        if (val > 32767)  val = 32767;
        if (val < -32768) val = -32768;
        output[i] = (short)val;
    }
}
