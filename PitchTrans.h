#ifndef VOL_TRANS_H
#define VOL_TRANS_H

#include <ap_fixed.h>

#define SAMPLE_SIZE 1024
typedef ap_fixed<16, 4> fixed_t;


void pitch_shift(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], fixed_t tones);
#endif