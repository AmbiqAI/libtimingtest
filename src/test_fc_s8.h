#ifndef __TEST_FC_S8_H
#define __TEST_FC_S8_H

#include <stdint.h>
#include "arm_math.h"


uint32_t
test_fc_s8_init();

uint32_t
test_fc_s8_run(int8_t *input, int8_t *output);

#endif // __TEST_FC_S8_H
