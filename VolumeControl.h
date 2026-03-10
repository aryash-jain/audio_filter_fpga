#ifndef VOL_CTRL_H
#define VOL_CTRL_H

#include <ap_fixed.h>

#define SAMPLE_SIZE 1024
typedef ap_fixed<16, 4> gain_t;

void change_volume(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], gain_t coeff);
#endif