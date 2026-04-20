#include "AudioMain.h"


static void copy(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE]) {
    for (int i = 0; i < SAMPLE_SIZE; i++) {
#pragma HLS UNROLL factor=4
        output[i] = input[i];
    }
}

void audio_main(audio_stream_t &input, audio_stream_t &output,
                enable_t en, gain_t vol, sample_t delay,
                sample_t decay, short tones) {

#pragma HLS INTERFACE axis      port=input
#pragma HLS INTERFACE axis      port=output
#pragma HLS INTERFACE s_axilite port=en
#pragma HLS INTERFACE s_axilite port=vol
#pragma HLS INTERFACE s_axilite port=delay
#pragma HLS INTERFACE s_axilite port=decay
#pragma HLS INTERFACE s_axilite port=tones
#pragma HLS INTERFACE s_axilite port=return

    short buf_in[SAMPLE_SIZE];
    for (int i = 0; i < SAMPLE_SIZE; i++) {
#pragma HLS PIPELINE II=1
        audio_sample_t s = input.read();
        buf_in[i] = (short)s.data;
    }

    short buf_a[SAMPLE_SIZE];
    short buf_b[SAMPLE_SIZE];
    short buf_out[SAMPLE_SIZE];
#pragma HLS ARRAY_PARTITION variable=buf_a cyclic factor=4 dim=1
#pragma HLS ARRAY_PARTITION variable=buf_b cyclic factor=4 dim=1
#pragma HLS ARRAY_PARTITION variable=buf_out cyclic factor=4 dim=1

    if (en[0]) change_volume(buf_in, buf_a, vol);
    else copy(buf_in, buf_a);

    if (en[1]) echo_control(buf_a, buf_b, delay, decay);
    else copy(buf_a, buf_b);

    if (en[2]) pitch_shift(buf_b, buf_out, tones);
    else copy(buf_b, buf_out);

    for (int i = 0; i < SAMPLE_SIZE; i++) {
#pragma HLS PIPELINE II=1
        audio_sample_t s;
        s.data = buf_out[i];
        s.last = (i == SAMPLE_SIZE - 1);
        s.keep = -1;
        s.strb = -1;
        output.write(s);
    }
}
