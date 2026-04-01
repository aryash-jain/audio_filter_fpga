#include <cstdio>
#include <cmath>
#include <cstdlib>
#include "AudioMain.h"

#define FS   48000
#define FREQ 440

static void gen_sine(short buf[SAMPLE_SIZE], double freq, double amp) {
    for (int i = 0; i < SAMPLE_SIZE; i++)
        buf[i] = (short)(amp * sin(2.0 * M_PI * freq * i / FS));
}

static void find_peak_freq(short buf[SAMPLE_SIZE], double fs, double *peak_freq) {
    double max_mag = 0;
    int peak_bin = 0;
    for (int k = 1; k < SAMPLE_SIZE / 2; k++) {
        double re = 0, im = 0;
        for (int n = 0; n < SAMPLE_SIZE; n++) {
            double ang = 2.0 * M_PI * k * n / SAMPLE_SIZE;
            re += buf[n] * cos(ang);
            im -= buf[n] * sin(ang);
        }
        double mag = sqrt(re * re + im * im);
        if (mag > max_mag) { max_mag = mag; peak_bin = k; }
    }
    *peak_freq = (double)peak_bin * fs / SAMPLE_SIZE;
}

static int test_passthrough() {
    printf("\n--- Passthrough (enable=0b000) ---\n");
    short in[SAMPLE_SIZE], out[SAMPLE_SIZE];
    int err = 0;

    gen_sine(in, FREQ, 16000);
    enable_t en = 0b000;
    audio_main(in, out, en, (gain_t)1.0, (sample_t)0, (sample_t)0, (short)0);

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
    enable_t en = 0b001;

    /* half gain */
    audio_main(in, out, en, (gain_t)0.5, (sample_t)0, (sample_t)0, (short)0);
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        short expected = (short)(in[i] * 0.5);
        if (abs(out[i] - expected) > 1) { err++; break; }
    }
    printf("  half gain: %s\n", err ? "FAIL" : "PASS");

    /* clipping at 2× */
    int e2 = 0;
    audio_main(in, out, en, (gain_t)2.0, (sample_t)0, (sample_t)0, (short)0);
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        if (out[i] > 32627 || out[i] < -32628) { e2++; break; }
    }
    printf("  clip 2x:   %s\n", e2 ? "FAIL" : "PASS");
    err += e2;

    return err;
}

static int test_echo_only() {
    printf("\n--- Echo only (enable=0b010) ---\n");
    short in[SAMPLE_SIZE], out[SAMPLE_SIZE];
    int err = 0;
    enable_t en = 0b010;

    // impulse: echo should appear at sample 'delay'
    for (int i = 0; i < SAMPLE_SIZE; i++) in[i] = 0;
    in[0] = 10000;

    audio_main(in, out, en, (gain_t)1.0, (sample_t)4, (sample_t)0.5, (short)0);

    if (out[0] != 10000) {
        printf("  impulse[0]:  FAIL (got %d)\n", out[0]);
        err++;
    } else {
        printf("  impulse[0]:  PASS\n");
    }

    short exp_echo = (short)(10000 * 0.5);
    if (abs(out[4] - exp_echo) > 1) {
        printf("  echo tap[4]: FAIL (got %d, expected ~%d)\n", out[4], exp_echo);
        err++;
    } else {
        printf("  echo tap[4]: PASS\n");
    }

    // decay=0 → passthrough 
    int e2 = 0;
    gen_sine(in, FREQ, 8000);
    audio_main(in, out, en, (gain_t)1.0, (sample_t)50, (sample_t)0.0, (short)0);
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
    enable_t en = 0b100;
    double f_in = 1000.0;
    double f_out;
    double bin_res = (double)FS / SAMPLE_SIZE;

    // no shift 
    gen_sine(in, f_in, 8000);
    audio_main(in, out, en, (gain_t)1.0, (sample_t)0, (sample_t)0, (short)0.0);
    find_peak_freq(out, FS, &f_out);

    if (fabs(f_out - f_in) > bin_res * 1.5) {
        printf("  no-shift:    FAIL (%.1f -> %.1f Hz)\n", f_in, f_out);
        err++;
    } else {
        printf("  no-shift:    PASS (%.1f -> %.1f Hz)\n", f_in, f_out);
    }

    // octave up
    gen_sine(in, f_in, 8000);
    audio_main(in, out, en, (gain_t)1.0, (sample_t)0, (sample_t)0, (short)12.0);
    find_peak_freq(out, FS, &f_out);
    double expected = f_in * 2.0;

    if (fabs(f_out - expected) > bin_res * 2.0) {
        printf("  octave up:   FAIL (expected ~%.1f, got %.1f Hz)\n", expected, f_out);
        err++;
    } else {
        printf("  octave up:   PASS (%.1f Hz)\n", f_out);
    }

    return err;
}


static int test_full_chain() {
    printf("\n--- Full chain: Vol→Echo→Pitch (enable=0b111) ---\n");
    short in[SAMPLE_SIZE], out[SAMPLE_SIZE];
    int err = 0;
    enable_t en = 0b111;

    gen_sine(in, 1000.0, 8000);

    audio_main(in, out, en,
                   (gain_t)0.8,       /* vol  */
                   (sample_t)10,      /* echo delay */
                   (sample_t)0.3,     /* echo decay */
                   (fixed_t)0.0);     /* pitch: no shift */

    /* basic sanity: output should not be silent */
    int nonzero = 0;
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        if (out[i] != 0) nonzero++;
    }
    if (nonzero < SAMPLE_SIZE / 4) {
        printf("  liveness:    FAIL (too many zeros: %d/%d)\n",
               SAMPLE_SIZE - nonzero, SAMPLE_SIZE);
        err++;
    } else {
        printf("  liveness:    PASS (%d/%d nonzero)\n", nonzero, SAMPLE_SIZE);
    }

    /* clip check */
    int e2 = 0;
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        if (out[i] > 32767 || out[i] < -32768) { e2++; break; }
    }
    printf("  clip check:  %s\n", e2 ? "FAIL" : "PASS");
    err += e2;

    // frequency sanity: pitch_tones=0 so peak should still be near 1 kHz
    double f_out;
    find_peak_freq(out, FS, &f_out);
    double bin_res = (double)FS / SAMPLE_SIZE;
    if (fabs(f_out - 1000.0) > bin_res * 2.0) {
        printf("  freq check:  FAIL (expected ~1000, got %.1f Hz)\n", f_out);
        err++;
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
