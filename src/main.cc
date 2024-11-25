/**
 * @file main.c
 * @author Adam Page (adam.page@ambiq.com)
 * @version 1.0
 * @date 2024-12-25
 *
 * @copyright Copyright (c) 2024
 *
 */


#include <arm_mve.h>

#include "main.h"
#include "arm_leaky_relu.h"

#define STIMULUS_LEN 1024
static float32_t data[STIMULUS_LEN];

int
main(void)
{
    // Initialize data with random values
    for (int i = 0; i < STIMULUS_LEN; i++)
    {
        data[i] = (float32_t)rand() / RAND_MAX;
    }
    arm_leaky_relu_f32(data, STIMULUS_LEN, 0.1);
    while (1) {};
}
