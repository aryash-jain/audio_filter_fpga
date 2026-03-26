#include "AudioMain.h"


static void copy(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE]) {
    for (int i = 0; i < SAMPLE_SIZE; i++) {
#pragma HLS PIPELINE II=1
        output[i] = input[i];
    }
}

void main(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], enable_t en, gain_t vol, sample_t delay, sample_t decay, fixed_t tones) {
#pragma HLS INTERFACE axis      port=input
#pragma HLS INTERFACE axis      port=output
#pragma HLS INTERFACE s_axilite port=enable
#pragma HLS INTERFACE s_axilite port=vol_coeff
#pragma HLS INTERFACE s_axilite port=echo_delay
#pragma HLS INTERFACE s_axilite port=echo_decay
#pragma HLS INTERFACE s_axilite port=pitch_tones
#pragma HLS INTERFACE s_axilite port=return
    
    short buf_a[SAMPLE_SIZE];
    short buf_b[SAMPLE_SIZE];

    if (en[0]) // volume control ON
        change_volume(input, buf_a, vol);
    else // volume control OFF, copy input to buf_a
        copy(input, buf_a);
    
    if (en[1]) // echo control ON
        echo_control(buf_a, buf_b, delay, decay);
    else // echo control OFF, copy buf_a to buf_b
        copy(buf_a, buf_b);
    
    if (en[2]) // transcription control ON
        pitch_shift(buf_b, output, tones);
    else // transcription control OFF, copy buf_b into output
        copy(buf_b, output);

}