#include "PitchTrans.h"
#include "fft.h"
#include <ap_fixed.h>
#include <hls_math.h>

typedef ap_fixed<24, 3>  phase_t;
typedef ap_fixed<16, 1>  mag_t;
typedef ap_fixed<16, 3>  ratio_t;

static const float PI     = 3.14159265358979f;
static const float TWO_PI = 6.28318530717959f;

static const accum_t OLA_NORM = (accum_t)((2.0f * (float)HOP_SIZE) / (float)FRAME_SIZE);

void pitch_shift(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], short tones)
{
    const int N = FRAME_SIZE;   // 512
    const int H = HOP_SIZE;     // 256

    // Control path only — float fine here
    ratio_t ratio = (ratio_t)hls::powf(2.0f, (float)tones / 12.0f);

    // Hann window — computed once outside hop loop
    fixed_t hann[FRAME_SIZE];
#pragma HLS ARRAY_PARTITION variable=hann cyclic factor=4 dim=1
    for (int n = 0; n < N; n++) {
#pragma HLS UNROLL factor=4
        float angle = TWO_PI * (float)n / (float)(N - 1);
        hann[n] = (fixed_t)(0.5f * (1.0f - hls::cosf(angle)));
    }

    // Normalise input: short → fixed_t [-1, 1)
    fixed_t norm_in[SAMPLE_SIZE];
#pragma HLS ARRAY_PARTITION variable=norm_in cyclic factor=4 dim=1
    for (int i = 0; i < SAMPLE_SIZE; i++) {
#pragma HLS UNROLL factor=4
        norm_in[i] = (fixed_t)((float)input[i] * (1.0f / 32768.0f));
    }

    // Output accumulator — cleared once
    accum_t acc[SAMPLE_SIZE];
#pragma HLS ARRAY_PARTITION variable=acc cyclic factor=4 dim=1
    for (int j = 0; j < SAMPLE_SIZE; j++) {
#pragma HLS UNROLL factor=4
        acc[j] = (accum_t)0;
    }

    // Phase state — fixed-point, persists across hops
    phase_t phase_in [FRAME_SIZE / 2];
    phase_t phase_out[FRAME_SIZE / 2];
    for (int i = 0; i < N / 2; i++) {
#pragma HLS PIPELINE II=1
        phase_in[i]  = (phase_t)0;
        phase_out[i] = (phase_t)0;
    }

    // Precompute expected advance per bin: 2π * m * H / N — constant across hops
    phase_t expected_adv[FRAME_SIZE / 2];
    for (int m = 0; m < N / 2; m++) {
#pragma HLS PIPELINE II=1
        expected_adv[m] = (phase_t)(TWO_PI * (float)m * (float)H / (float)N);
    }

    fixed_t  frame[FRAME_SIZE];
    fixed_t  outFrame[FRAME_SIZE];
#pragma HLS ARRAY_PARTITION variable=frame    cyclic factor=4 dim=1
#pragma HLS ARRAY_PARTITION variable=outFrame cyclic factor=4 dim=1
    cmpxData spec[FRAME_SIZE];
    cmpxData shiftSpec[FRAME_SIZE];
#pragma HLS ARRAY_PARTITION variable=spec      cyclic factor=4 dim=1
#pragma HLS ARRAY_PARTITION variable=shiftSpec cyclic factor=4 dim=1
    // Hop loop — 5 hops (k = -H, 0, H, 2H, 3H) gives full 2-frame overlap
    // for all SAMPLE_SIZE=1024 output samples. Negative/out-of-bounds input
    // indices are zero-padded.
    for (int k = -H; k < SAMPLE_SIZE; k += H) {
#pragma HLS LOOP_TRIPCOUNT min=5 max=5

        // Analysis window
        for (int j = 0; j < N; j++) {
#pragma HLS UNROLL factor=4
            int idx = k + j;
            frame[j] = (idx >= 0 && idx < SAMPLE_SIZE)
                       ? (fixed_t)(norm_in[idx] * hann[j])
                       : (fixed_t)0;
        }

        fft_wrapper(frame, spec);

        for (int i = 0; i < N; i++) {
#pragma HLS UNROLL factor=4
            shiftSpec[i] = cmpxData(0, 0);
        }

        shiftSpec[0]   = spec[0];
        shiftSpec[N/2] = spec[N/2];

        // Track dominant (max-magnitude) input bin per output bin so that
        // phase_out[newK] reflects the strongest contributor, not last-write-wins.
        mag_t dominant_mag[FRAME_SIZE / 2];
        for (int i = 0; i < N / 2; i++) {
#pragma HLS PIPELINE II=1
            dominant_mag[i] = (mag_t)0;
        }

        for (int m = 1; m < N / 2; m++) {
#pragma HLS PIPELINE II=1

            int newK = (int)((float)m * (float)ratio + 0.5f);
            if (newK > 0 && newK < N / 2) {
                float re_f    = (float)spec[m].real();
                float im_f    = (float)spec[m].imag();
                mag_t   mag   = (mag_t)hls::sqrt(re_f * re_f + im_f * im_f);
                phase_t phase = (phase_t)hls::atan2f(im_f, re_f);

                // Phase delta with expected-advance correction
                float delta_f = (float)phase
                              - (float)phase_in[m]
                              - (float)expected_adv[m];

                // Wrap delta to [-π, π]
                delta_f -= TWO_PI * hls::floorf((delta_f + PI) / TWO_PI);

                phase_in[m] = phase;

                // Accumulate and wrap output phase
                float pout_f = (float)phase_out[newK]
                             + (float)expected_adv[m] * (float)ratio
                             + delta_f * (float)ratio;

                pout_f -= TWO_PI * hls::floorf((pout_f + PI) / TWO_PI);

                // Only the dominant (largest magnitude) bin drives phase_out
                if (mag > dominant_mag[newK]) {
                    dominant_mag[newK] = mag;
                    phase_out[newK] = (phase_t)pout_f;
                }

                fixed_t new_re = (fixed_t)((float)mag * hls::cosf(pout_f));
                fixed_t new_im = (fixed_t)((float)mag * hls::sinf(pout_f));

                // Accumulate — fixes bin collision overwrite
                shiftSpec[newK] = cmpxData(
                    shiftSpec[newK].real() + new_re,
                    shiftSpec[newK].imag() + new_im
                );

                // Conjugate symmetry for real IFFT
                shiftSpec[N - newK] = cmpxData(
                     shiftSpec[newK].real(),
                    -shiftSpec[newK].imag()
                );
            }
        }

        ifft_wrapper(shiftSpec, outFrame);

        // OLA accumulate — guard writes past end of buffer
        for (int j = 0; j < N; j++) {
#pragma HLS UNROLL factor=4
            int idx = k + j;
            if (idx >= 0 && idx < SAMPLE_SIZE)
                acc[idx] += (accum_t)(outFrame[j] * hann[j]) * OLA_NORM;
        }
    }

    // Convert accumulator to int16.
    // Cast to float FIRST before *32768 — multiplying inside ap_fixed<32,8>
    // overflows immediately since its max value is only 127.999,
    // causing the entire output to saturate to zero.
    for (int i = 0; i < SAMPLE_SIZE; i++) {
#pragma HLS UNROLL factor=4
        float fval = (float)acc[i] * 32768.0f;
        if (fval >  32767.0f) fval =  32767.0f;
        if (fval < -32768.0f) fval = -32768.0f;
        output[i] = (short)fval;
    }
}
