#ifndef aot_fully_connected_29_h
#define aot_fully_connected_29_h

#include <stdint.h>
#include "arm_nnfunctions.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the operator.
int32_t aot_fully_connected_29_init(void);

// Run the operator.
// @param input  Pointer to the input buffer.
// @param output Pointer to the output buffer.
int32_t aot_fully_connected_29_run(const int8_t* __restrict input, int8_t* __restrict output);

#ifdef __cplusplus
}
#endif

#endif // aot_fully_connected_29_h
