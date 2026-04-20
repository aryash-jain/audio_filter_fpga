#include <cstdio>
#include <cmath>
#include <cstdlib>
#include "AudioMain.h"

#define FS   48000
#define FREQ 440

static void run_audio(short in[SAMPLE_SIZE], short out[SAMPLE_SIZE],
                      enable_t en, gain_t vol, sample_t delay,
                      sample_t decay, short tones) {
    audio_stream_t in_stream, out_stream;

    for (int i = 0; i < SAMPLE_SIZE; i++) {
        audio_sample_t s;
        s.data = in[i];
        s.last = (i == SAMPLE_SIZE - 1);
        s.keep = -1;
        s.strb = -1;
        in_stream.write(s);
    }

    audio_main(in_stream, out_stream, en, vol, delay, decay, tones);

    for (int i = 0; i < SAMPLE_SIZE; i++) {
        audio_sample_t s = out_stream.read();
        out[i] = (short)s.data;
    }
}

static void gen_sine(short buf[SAMPLE_SIZE], double freq, double amp) {
    for (int i = 0; i < SAMPLE_SIZE; i++)
        buf[i] = (short)(amp * sin(2.0 * M_PI * freq * i / FS));
}

// Search only valid OLA region: samples FRAME_SIZE/2 to SAMPLE_SIZE-FRAME_SIZE/2
// Hann window tapers both edges to near-zero, DFT over those edges gives DC bias
static void find_peak_freq(short buf[SAMPLE_SIZE], double fs, double *peak_freq) {
    int start = FRAME_SIZE / 2;
    int end   = SAMPLE_SIZE - FRAME_SIZE / 2;
    int len   = end - start;

    double max_mag = 0;
    int peak_bin = 0;

    for (int k = 1; k < len / 2; k++) {
        double re = 0, im = 0;
        for (int n = 0; n < len; n++) {
            double ang = 2.0 * M_PI * k * n / len;
            re += buf[start + n] * cos(ang);
            im -= buf[start + n] * sin(ang);
        }
        double mag = sqrt(re * re + im * im);
        if (mag > max_mag) { max_mag = mag; peak_bin = k; }
    }
    *peak_freq = (double)peak_bin * fs / len;
}

static int test_passthrough() {
    printf("\n--- Passthrough (enable=0b000) ---\n");
    short in[SAMPLE_SIZE], out[SAMPLE_SIZE];
    int err = 0;

    gen_sine(in, FREQ, 16000);
    run_audio(in, out, 0b000, (gain_t)1.0, (sample_t)0, (sample_t)0, (short)0);

    for (int i = 0; i < SAMPLE_SIZE; i++) {
        if (abs(out[i] - in[i]) > 1) { err++; break; }
    }
    printf("  result: %s\n", err ? "FAIL" : "PASS");
    return err;
}

static int test_volume_only() {
    printf("\n--- Volume only (enable=0b001) ---\n");
    short in[SAMPLE_SIZE], out[SAMPLE_SIZE];
    int err = 0;

    gen_sine(in, FREQ, 16000);

    run_audio(in, out, 0b001, (gain_t)0.5, (sample_t)0, (sample_t)0, (short)0);
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        short expected = (short)(in[i] * 0.5);
        if (abs(out[i] - expected) > 1) { err++; break; }
    }
    printf("  half gain: %s\n", err ? "FAIL" : "PASS");

    int e2 = 0;
    run_audio(in, out, 0b001, (gain_t)2.0, (sample_t)0, (sample_t)0, (short)0);
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        if (out[i] > 32767 || out[i] < -32768) { e2++; break; }
    }
    printf("  clip 2x:   %s\n", e2 ? "FAIL" : "PASS");
    err += e2;

    return err;
}

static int test_echo_only() {
    printf("\n--- Echo only (enable=0b010) ---\n");
    short in[SAMPLE_SIZE], out[SAMPLE_SIZE];
    int err = 0;

    for (int i = 0; i < SAMPLE_SIZE; i++) in[i] = 0;
    in[0] = 10000;

    run_audio(in, out, 0b010, (gain_t)1.0, (sample_t)4, (sample_t)0.5, (short)0);

    if (out[0] != 10000) {
        printf("  impulse[0]:  FAIL (got %d)\n", out[0]); err++;
    } else {
        printf("  impulse[0]:  PASS\n");
    }

    short exp_echo = (short)(10000 * 0.5);
    if (abs(out[4] - exp_echo) > 1) {
        printf("  echo tap[4]: FAIL (got %d, expected ~%d)\n", out[4], exp_echo); err++;
    } else {
        printf("  echo tap[4]: PASS\n");
    }

    int e2 = 0;
    gen_sine(in, FREQ, 8000);
    run_audio(in, out, 0b010, (gain_t)1.0, (sample_t)50, (sample_t)0.0, (short)0);
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        if (abs(out[i] - in[i]) > 1) { e2++; break; }
    }
    printf("  no-echo:     %s\n", e2 ? "FAIL" : "PASS");
    err += e2;

    return err;
}

static int test_pitch_only() {
    printf("\n--- Pitch only (enable=0b100) ---\n");
    short in[SAMPLE_SIZE], out[SAMPLE_SIZE];
    int err = 0;
    double f_in  = 1000.0;
    double f_out;

    // bin resolution of the valid OLA region
    int    valid_len = SAMPLE_SIZE - FRAME_SIZE;   // 1024
    double bin_res   = (double)FS / valid_len;      // 46.9 Hz

    // --- no-shift ---
    gen_sine(in, f_in, 8000);
    run_audio(in, out, 0b100, (gain_t)1.0, (sample_t)0, (sample_t)0, (short)0);

    short peak = 0;
    for (int i = FRAME_SIZE/2; i < SAMPLE_SIZE - FRAME_SIZE/2; i++)
        if (abs(out[i]) > abs(peak)) peak = out[i];

    find_peak_freq(out, FS, &f_out);
    printf("  no-shift peak=%d  detected=%.1f Hz\n", peak, f_out);

    if (abs(peak) < 100) {
        printf("  no-shift:    FAIL (output too quiet)\n"); err++;
    } else if (fabs(f_out - f_in) > bin_res * 2.0) {
        printf("  no-shift:    FAIL (%.1f -> %.1f Hz)\n", f_in, f_out); err++;
    } else {
        printf("  no-shift:    PASS (%.1f -> %.1f Hz)\n", f_in, f_out);
    }

    // --- octave up ---
    gen_sine(in, f_in, 8000);
    run_audio(in, out, 0b100, (gain_t)1.0, (sample_t)0, (sample_t)0, (short)12);

    peak = 0;
    for (int i = FRAME_SIZE/2; i < SAMPLE_SIZE - FRAME_SIZE/2; i++)
        if (abs(out[i]) > abs(peak)) peak = out[i];

    find_peak_freq(out, FS, &f_out);
    double expected_freq = f_in * 2.0;
    printf("  octave up peak=%d  detected=%.1f Hz  expected=%.1f Hz\n",
           peak, f_out, expected_freq);

    if (abs(peak) < 100) {
        printf("  octave up:   FAIL (output too quiet)\n"); err++;
    } else if (fabs(f_out - expected_freq) > bin_res * 4.0) {
        printf("  octave up:   FAIL (expected ~%.1f, got %.1f Hz)\n",
               expected_freq, f_out); err++;
    } else {
        printf("  octave up:   PASS (%.1f Hz)\n", f_out);
    }

    return err;
}

static int test_full_chain() {
    printf("\n--- Full chain: Vol->Echo->Pitch (enable=0b111) ---\n");
    short in[SAMPLE_SIZE], out[SAMPLE_SIZE];
    int err = 0;

    gen_sine(in, 1000.0, 8000);
    run_audio(in, out, 0b111, (gain_t)0.8, (sample_t)10, (sample_t)0.3, (short)0);

    int nonzero = 0;
    int valid_len = SAMPLE_SIZE - FRAME_SIZE;
    for (int i = FRAME_SIZE/2; i < SAMPLE_SIZE - FRAME_SIZE/2; i++)
        if (out[i] != 0) nonzero++;

    if (nonzero < valid_len / 4) {
        printf("  liveness:    FAIL (too many zeros: %d/%d)\n",
               valid_len - nonzero, valid_len); err++;
    } else {
        printf("  liveness:    PASS (%d/%d nonzero)\n", nonzero, valid_len);
    }

    int e2 = 0;
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        if (out[i] > 32767 || out[i] < -32768) { e2++; break; }
    }
    printf("  clip check:  %s\n", e2 ? "FAIL" : "PASS");
    err += e2;

    double f_out;
    find_peak_freq(out, FS, &f_out);
    double bin_res = (double)FS / valid_len;
    if (fabs(f_out - 1000.0) > bin_res * 2.0) {
        printf("  freq check:  FAIL (expected ~1000, got %.1f Hz)\n", f_out); err++;
    } else {
        printf("  freq check:  PASS (%.1f Hz)\n", f_out);
    }

    return err;
}

int main() {
    int total = 0;
    total += test_passthrough();
    total += test_volume_only();
    total += test_echo_only();
    total += test_pitch_only();
    total += test_full_chain();

    printf("\n========================================\n");
    if (total == 0)
        printf("  ALL TESTS PASSED\n");
    else
        printf("  TOTAL ERRORS: %d\n", total);
    printf("========================================\n");
    return total ? 1 : 0;
}
