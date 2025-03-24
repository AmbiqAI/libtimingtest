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

 #ifndef __MODEL_AD_H
 #define __MODEL_AD_H

 #include <stdint.h>
 #include "arm_math.h"


 /**
  * @brief Initialize model
  *
  * @return uint32_t
  */
 uint32_t
 model_ad_init();

 /**
  * @brief Run model
  *
  * @return uint32_t
  */
 uint32_t
 model_ad_run(const int8_t *input, int8_t *output);

 #endif // __MODEL_AD_H
