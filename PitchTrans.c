#include "PitchTrans.h"
#include <hls_math.h>
#include <ap_fixed.h>
using namespace hls;


typedef struct {
    fixed_t real;
    fixed_t imag;
} complex_t;


void pitch_shift(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], fixed_t tones){

    //will be 1024
    const int N = FRAME_SIZE;
    //will be 256
    const int H = HOP_SIZE;

    fixed_t ratio = powf(2.0f, tones/ 12.0f);

    fixed_t hann[FRAME_SIZE];
    fixed_t frame[FRAME_SIZE];
    fixed_t output[FRAME_SIZE];

    
    complex_t spec[FRAME_SIZE];
    complex_t shiftSpec[FRAME_SIZE];

    //hann window
    for (int i = 0; i < N; i++){
        fixed_t wave = cos((fixed_t)(2.0 * M_PI * n/(N - 1)))
        hann[n] = (fixed_t)0.5 * ((fixed_t)1.0 - wave);
    }

    //output vect
    for (int j = 0; j < SAMPLE_SIZE; j++){
        output[j] = 0;
    }

    for (int k = 0; k < SAMPLE_SIZE - N; i += H){
        #pragma HLS PIPELINE II = 1
        frame[j] = (fixed_t)input[i+j] * hann[j];
    }

    //fft(frame,spec);

    for(int j = 0; j<N; j++) {
        #pragma HLS PIPELINE II = 1
        shiftSpec[j].real = 0;
        shiftSpec[j].imag = 0;
    }

    for (int k = 0; k < N/2; k++) {
        #pragma HLS PIPELINE II = 1

        int newK = (int)(k* ratio + 0.5);

        if(newK > 0 && newK < N/2){
            shiftSpec[newK].real += spec[k].real;
            shiftSpec [newK].imag += spec[k].imag;

            shiftSpec[N - newK].real += spec[N -k].real;
            shiftSpec[N - newK].imag += spec[N-k].imag;

        }


    }

    //ifft(shiftSpec, outFrame);


    //overlap-add
    for (int y = 0; y < N; y++){
        #pragma HLS PIPELINE II = 1

        accum_t val = (accum_t)outFrame[j] * hann[j];
        accum_t sum = (accum_t)output[i+j]+ val;



        // Clamp to short range
        if (sum > 32767) sum = 32767;
        if (sum < -32768) sum = -32768;

        output[i + j] = (short)sum;





    }





}



