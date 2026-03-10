#include "VolumeControl.h"

void change_volume(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], gain_t coeff) {
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        #pragma HLS PIPELINE II=1
        output[i] = (short) (input[i] * coeff);
    }
}
