#include "VolumeControl.h"

void change_volume(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], gain_t coeff) {
#pragma HLS INTERFACE axis port=input
#pragma HLS INTERFACE axis port=output
#pragma HLS INTERFACE s_axilite port=coeff
#pragma HLS INTERFACE s_axilite port=return
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        #pragma HLS PIPELINE II=1
        gain_t temp = input[i] * coeff;
        if (temp > 32627)
            temp = 32627;
        else if (temp < -32628)
            temp = -32628;
        output[i] = (short) temp;
    }
}
