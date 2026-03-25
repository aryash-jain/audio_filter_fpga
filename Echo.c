#include "Echo.h"

void echo_control(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], sample_t delay, sample_t decay) 
{
    #pragma HLS INTERFACE s_axilite port=input
    #pragma HLS INTERFACE s_axilite port=output
    #pragma HLS INTERFACE s_axilite port=delay
    #pragma HLS INTERFACE s_axilite port=decay
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS PIPELINE II=1


    for (int i = 0; i < SAMPLE_SIZE; i++) 
    {
        sample_t sum = input[i];
        if (i >= delay)
        {
            sum += input[i - delay] * decay;
            if (sum > 32627)
            {
                sum = 32627;
            }
            else if (sum < -32628)
            {
                sum = -32628;
            }
            output[i] = (short) sum;
        }
    }
}