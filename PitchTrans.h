#ifndef PITCH_TRANS_H
#define PITCH_TRANS_H

#include <ap_fixed.h>

#define SAMPLE_SIZE 1024
#define FRAME_SIZE 1024
#define HOP_SIZE 256

typedef ap_fixed<16, 4> fixed_t;
typedef ap_fixed<32, 8> accum_t;

void pitch_shift(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], fixed_t tones);

#endif