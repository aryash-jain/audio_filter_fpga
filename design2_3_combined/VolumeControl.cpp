#include "VolumeControl.h"

void change_volume(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], gain_t coeff) {
#pragma HLS ARRAY_PARTITION variable=input  cyclic factor=4 dim=1
#pragma HLS ARRAY_PARTITION variable=output cyclic factor=4 dim=1
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        #pragma HLS UNROLL factor=4
        gain_t temp = input[i] * coeff;
        if (temp > 32627)
            temp = 32627;
        else if (temp < -32628)
            temp = -32628;
        output[i] = (short) temp;
    }
}
