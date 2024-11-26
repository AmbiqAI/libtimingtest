#ifndef __ARM_LEAKY_RELU_H
#define __ARM_LEAKY_RELU_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <arm_math.h>

void
arm_leaky_relu_f32(float32_t *data, int32_t size, float32_t alpha);
#ifdef __cplusplus
}
#endif

#endif // __ARM_LEAKY_RELU_H
