#ifndef ECHO_H
#define ECHO_H

#include "ap_fixed.h"

#define SAMPLE_SIZE 1024
typedef ap_fixed<24, 16> sample_t;

void echo_control(sample_t input, sample_t output, short delay, short decay);

#endif