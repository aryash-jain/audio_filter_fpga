#include <cstdio>
#include <cmath>
#include <cstdlib>

#include "AudioMain.h"

#define FS 48000
#define FREQ 440 //A4

static void gen_sig(short buf[SAMPLE_SIZE], double freq, double vol) { // create test signal
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        buf[i] = (short) (vol * sin(2.0 * M_PI * freq * i / FS));
    }
}