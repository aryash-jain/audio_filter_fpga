#include "fft.h"

struct fft_config : hls::ip_fft::params_t {
    static const bool has_nfft = false;
    static const unsigned input_width = 24;
    static const unsigned output_width = 24;
    static const unsigned status_width = 8;
    static const unsigned config_width = 16;
    static const unsigned max_nfft = 10;
    static const unsigned stages_block_ram = 0;
    static const unsigned arch_opt = hls::ip_fft::pipelined_streaming_io;
    static const unsigned ordering_opt = hls::ip_fft::natural_order;
    static const unsigned scaling_opt = hls::ip_fft::scaled;
    static const unsigned rounding_opt = hls::ip_fft::convergent_rounding;
};

typedef hls::ip_fft::config_t<fft_config> fft_runtime_config_t;
typedef hls::ip_fft::status_t<fft_config> fft_status_t;

void fft_wrapper(fixed_t in[FRAME_SIZE], cmpxData out[FRAME_SIZE]) {
    fft_cmpx_t xn[FRAME_SIZE];
    fft_cmpx_t xk[FRAME_SIZE];
    fft_status_t status;
    fft_runtime_config_t config;

    config.setDir(1);
    config.setSch(0xAAAAA); 

    for (int i = 0; i < FRAME_SIZE; i++) {
#pragma HLS PIPELINE II=1
        xn[i] = fft_cmpx_t((fft_data_t)in[i], (fft_data_t)0);
    }

    hls::fft<fft_config>(xn, xk, &status, &config);

    for (int i = 0; i < FRAME_SIZE; i++) {
#pragma HLS PIPELINE II=1
        fft_cmpx_t temp = xk[i];  

        fixed_t real_part = (fixed_t)temp.real();
        fixed_t imag_part = (fixed_t)temp.imag();

        out[i] = cmpxData(real_part, imag_part);
    }
}

void ifft_wrapper(cmpxData in[FRAME_SIZE], fixed_t out[FRAME_SIZE]) {
    fft_cmpx_t xn[FRAME_SIZE];
    fft_cmpx_t xk[FRAME_SIZE];
    fft_status_t status;
    fft_runtime_config_t config;

    config.setDir(0);
    config.setSch(0x00000);

    for (int i = 0; i < FRAME_SIZE; i++) {
#pragma HLS PIPELINE II=1
        xn[i] = fft_cmpx_t((fft_data_t)in[i].real(), (fft_data_t)in[i].imag());
    }

    hls::fft<fft_config>(xn, xk, &status, &config);

    for (int i = 0; i < FRAME_SIZE; i++) {
#pragma HLS PIPELINE II=1
        fft_cmpx_t temp = xk[i]; 

        out[i] = (fixed_t)temp.real();
    }
}
