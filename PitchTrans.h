#ifndef PITCH_TRANS_H
#define PITCH_TRANS_H

#include <ap_fixed.h>

#define FRAME_SIZE  512
#define HOP_SIZE    256
#define SAMPLE_SIZE 1024   // must be > FRAME_SIZE to get multiple hops
                           // 2048 = FRAME_SIZE + 4*HOP_SIZE → 5 hops

typedef ap_fixed<16, 4> fixed_t;
typedef ap_fixed<32, 8> accum_t;

// Internal phase-vocoder types
typedef ap_fixed<24, 3> phase_t;
typedef ap_fixed<16, 1> mag_t;
typedef ap_fixed<16, 3> ratio_t;

void pitch_shift(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], short tones);

#endif
