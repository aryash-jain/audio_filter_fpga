#include "Echo.h"

void echo_control(short input[SAMPLE_SIZE], short output[SAMPLE_SIZE], sample_t delay, sample_t decay) 
{
#pragma HLS ARRAY_PARTITION variable=input  cyclic factor=4 dim=1
#pragma HLS ARRAY_PARTITION variable=output cyclic factor=4 dim=1
    int d = (int)delay;

    for (int i = 0; i < SAMPLE_SIZE; i++) 
    {
#pragma HLS PIPELINE II=1
        sample_t sum = input[i];
        if (i >= d)
        {
            sum += output[i - d] * decay;
            if (sum > 32767)
                sum = 32767;
            else if (sum < -32768)
                sum = -32768;
        }
        output[i] = (short)sum;
    }
}
