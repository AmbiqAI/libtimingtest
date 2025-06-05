#ifndef aot_average_pool_27_h
#define aot_average_pool_27_h

#include <stdint.h>
#include "arm_nnfunctions.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the operation.
int32_t aot_average_pool_27_init(void);

// Run the operation.
// @param input  Pointer to the input buffer.
// @param output Pointer to the output buffer.
int32_t aot_average_pool_27_run(const int8_t* __restrict input, int8_t* __restrict output);

#ifdef __cplusplus
}
#endif

#endif // aot_average_pool_27_h
