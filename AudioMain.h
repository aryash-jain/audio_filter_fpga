#ifndef AUDIO_MAIN_H
#define AUDIO_MAIN_H

#include <ap_int.h>

#include "VolumeControl.h"
#include "Echo.h"
#include "PitchTrans.h"

#define SAMPLE_SIZE 1024
#define FRAME_SIZE 1024
#define HOP_SIZE 256

// 1st bit controls vol, 2nd echo, 3rd transcription. 3-bit structure allows for multiple simultaneous functions
typedef ap_uint<3> enable_t;

void audio_main(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], enable_t en, gain_t vol, sample_t delay, sample_t decay, fixed_t tones);

#endif