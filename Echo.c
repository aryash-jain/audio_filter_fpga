#include "Echo.h"

void echo_control(sample_t input, sample_t output, short delay_ms, short decay) 
{
    #pragma HLS PIPELINE II=1
    for (int i = 0; i < SAMPLE_SIZE; i++) 
    {
        output[i] = input[i];
        if (i >= delay)
        {
            output[i] += input[i - delay] * decay;
        }
    }
}