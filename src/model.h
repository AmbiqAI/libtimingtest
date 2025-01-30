/**
 * @file model.h
 * @author Adam Page (adam.page@ambiq.com)
 * @brief TFLM model
 * @version 1.0
 * @date 2025-01-30
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __DBG_MODEL_H
#define __DBG_MODEL_H

#include <stdint.h>
#include "arm_math.h"
#include "tflm.h"


/**
 * @brief Initialize model
 *
 * @return uint32_t
 */
uint32_t
model_init();

/**
 * @brief Run model
 *
 * @return uint32_t
 */
uint32_t
model_inference();

#endif // __DBG_MODEL_H
