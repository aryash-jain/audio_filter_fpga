#include "PitchTrans.h"
#include "fft.h"
#include <hls_math.h>

void pitch_shift(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], short tones) {
    const int N = FRAME_SIZE;
    const int H = HOP_SIZE;

    printf("input[100]=%d\n", input[100]);
    printf("input[1000]=%d\n", input[1000]);

    float exp_f = (float)tones / 12.0f;
    fixed_t ratio = (fixed_t)hls::powf(2.0f, exp_f);
    printf("ratio = %f\n", (float)ratio);


    fixed_t hann[FRAME_SIZE];
    for (int n = 0; n < N; n++) {
#pragma HLS PIPELINE II=1
        float angle = 2.0f * 3.14159265f * (float)n / (float)(N - 1);
        hann[n] = (fixed_t)(0.5f * (1.0f - hls::cosf(angle)));
    }


    fixed_t norm_in[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++) {
#pragma HLS PIPELINE II=1
        norm_in[i] = (fixed_t)((float)input[i] / 32768.0f);
    }


    accum_t acc[SAMPLE_SIZE];
    for (int j = 0; j < SAMPLE_SIZE; j++) {
#pragma HLS PIPELINE II=1
        acc[j] = 0;
    }

    fixed_t frame[FRAME_SIZE];
    fixed_t outFrame[FRAME_SIZE];
    cmpxData spec[FRAME_SIZE];
    cmpxData shiftSpec[FRAME_SIZE];

    for (int k = 0; k <= SAMPLE_SIZE - N; k += H) {


        for (int j = 0; j < N; j++) {
#pragma HLS PIPELINE II=1
            frame[j] = norm_in[k + j] * hann[j];
        }

        fft_wrapper(frame, spec);

        for (int i = 0; i < 10; i++) {

}


for (int i = 0; i < N; i++) {
#pragma HLS PIPELINE II=1
    shiftSpec[i] = cmpxData(0, 0);
}

shiftSpec[0] = spec[0];
shiftSpec[N/2] = spec[N/2];

for (int m = 1; m < N / 2; m++) {
#pragma HLS PIPELINE II=1
    int newK = (int)(m * ratio + (accum_t)0.5f);

    if (newK > 0 && newK < N / 2) {
        cmpxData val = spec[m];
        shiftSpec[newK] = val;
        shiftSpec[N - newK] = cmpxData(val.real(), -val.imag());
    }
}

        ifft_wrapper(shiftSpec, outFrame);


        for (int j = 0; j < N; j++) {
#pragma HLS PIPELINE II=1
            acc[k + j] += (accum_t)(outFrame[j] * hann[j]) * (accum_t)2.0;
        }
    }


for (int i = 0; i < SAMPLE_SIZE; i++) {
#pragma HLS PIPELINE II=1
    float fval = (float)acc[i] * 32768.0f;
    if (fval >  32767.0f) fval =  32767.0f;
    if (fval < -32768.0f) fval = -32768.0f;
    output[i] = (short)fval;
}



}
