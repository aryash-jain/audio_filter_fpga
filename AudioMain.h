#ifndef AUDIO_MAIN_H
#define AUDIO_MAIN_H

#include <ap_int.h>
#include "VolumeControl.h"
#include "Echo.h"
#include "PitchTrans.h"
#include "hls_stream.h"
#include "ap_axi_sdata.h"


#define SAMPLE_SIZE 1024
#define FRAME_SIZE 512
#define HOP_SIZE 256

// 1st bit controls vol, 2nd echo, 3rd transcription. 3-bit structure allows for multiple simultaneous functions
typedef ap_uint<3> enable_t;
typedef ap_axiu<16, 0, 0, 0> audio_sample_t;
typedef hls::stream<audio_sample_t> audio_stream_t;

void audio_main(audio_stream_t &input, audio_stream_t &output, enable_t en, gain_t vol, sample_t delay, sample_t decay, short tones);

#endif
