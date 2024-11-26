/**
 * @file main.c
 * @author Adam Page (adam.page@ambiq.com)
 * @version 1.0
 * @date 2024-12-25
 *
 * @copyright Copyright (c) 2024
 *
 */


#include "ns_ambiqsuite_harness.h"
#include "ns_peripherals_power.h"
#include <arm_mve.h>

#include "main.h"
#include "arm_leaky_relu.h"

#define STIMULUS_LEN 1024
static float32_t data[STIMULUS_LEN];


int
main(void)
{
   // Fill two arrays with random values
   float32_t A[10] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};
   float32_t B[10] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};
   float32_t Dst[10];

    // Initialize data with random values
    for (int i = 0; i < STIMULUS_LEN; i++)
    {
        data[i] = (float32_t)rand() / RAND_MAX;
    }
    arm_leaky_relu_f32(data, STIMULUS_LEN, 0.1);
    while (1) {};
}
