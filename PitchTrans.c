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

        


    }






}



