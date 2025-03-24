#ifndef __TEST_FC_16X8_H
#define __TEST_FC_16X8_H

#include <stdint.h>
#include "arm_math.h"


uint32_t
test_fc_16x8_init();

uint32_t
test_fc_16x8_run(int16_t *input, int16_t *output);

#endif // __TEST_FC_16X8_H
