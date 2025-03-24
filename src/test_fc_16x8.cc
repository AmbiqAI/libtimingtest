#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "ns_ambiqsuite_harness.h"
#include "arm_nnfunctions.h"  // CMSIS-NN API
#include "arm_nnsupportfunctions.h"  // CMSIS-NN API
#include "test_fc_16x8.h"

extern ns_timer_config_t timerCfg;

#define MAX_ACTIVATION_SIZE (1024)

#define INPUT_LEN (640)
#define OUTPUT_LEN (640)

alignas(16) static int8_t fc_weights[OUTPUT_LEN * INPUT_LEN] = {0};
alignas(16) static int64_t fc_bias[OUTPUT_LEN] = {0};
alignas(16) static int8_t ctx_buffer[1024] = {0};

alignas(16) static int16_t act_buffer0[MAX_ACTIVATION_SIZE];
alignas(16) static int16_t act_buffer1[MAX_ACTIVATION_SIZE];

static const cmsis_nn_dims input_dims = {
    .n = 1, .h = 0, .w = 0, .c = INPUT_LEN
};

static const cmsis_nn_dims fc_filter_dims = {
    .n = INPUT_LEN, .h = 0, .w = 0, .c = OUTPUT_LEN
};

static const cmsis_nn_dims fc_bias_dims   = {
    .n = 0, .h = 0, .w = 0, .c = OUTPUT_LEN
};

static const cmsis_nn_dims fc_output_dims = {
    .n = 1, .h = 0, .w = 0, .c = OUTPUT_LEN
};

static cmsis_nn_per_tensor_quant_params quant_params_fc = {
    .multiplier = 0, .shift = 1
};

static const cmsis_nn_fc_params fc_params_linear = {
    .input_offset  = 0,
    .filter_offset = 0,
    .output_offset = 0,
    .activation    = { .min = -32767, .max = 32767 }
};


 /**
  * @ingroup groupSupport
  */

 /**
  * @addtogroup supportFC
  * @{
  */

#define USE_CUSTOM_INTRINSIC 1

 /*
  * s16 vector(lhs) by s8 matrix (transposed) multiplication
  *
  * Refer header file for details.
  *
  */
 arm_cmsis_nn_status test_arm_nn_vec_mat_mult_t_s16(const int16_t *lhs,
                                               const int8_t *rhs,
                                               const int64_t *bias,
                                               int16_t *dst,
                                               const int32_t dst_multiplier,
                                               const int32_t dst_shift,
                                               const int32_t rhs_cols,
                                               const int32_t rhs_rows,
                                               const int32_t activation_min,
                                               const int32_t activation_max)
 {
 #if defined(ARM_MATH_DSP)

     int32_t rhs_cols_fast = rhs_cols;

     if (rhs_cols > MAX_COL_COUNT)
     {
         rhs_cols_fast = MAX_COL_COUNT;
     }

     #if defined(ARM_MATH_MVEI)
     int32_t row_loop_cnt = rhs_rows / 4;
#if defined(USE_CUSTOM_INTRINSIC)
#else
     int32_t col_loop_cnt = (rhs_cols_fast + 7) / 8;
#endif

     for (int32_t i_row_loop_count = 0; i_row_loop_count < row_loop_cnt; i_row_loop_count++)
     {

        int32_t result_0 = 0;
        int32_t result_1 = 0;
        int32_t result_2 = 0;
        int32_t result_3 = 0;

        const int16_t *lhs_ptr = lhs;
        register const int8_t *rhs_ptr_0 asm("r0") = rhs;
        register const int8_t *rhs_ptr_1 asm("r1") = rhs_ptr_0 + rhs_cols;
        register const int8_t *rhs_ptr_2 asm("r2") = rhs_ptr_1 + rhs_cols;
        register const int8_t *rhs_ptr_3 asm("r3") = rhs_ptr_2 + rhs_cols;
        // rhs = rhs_ptr_3 + rhs_cols;

#if defined(USE_CUSTOM_INTRINSIC)
        __ASM (
            " .p2align 2                                 \n"
            "   wlstp.16        lr, %[cnt], 1f           \n"
            "   mov             %[out0], 0               \n"
            "   mov             %[out1], 0               \n"
            "   mov             %[out2], 0               \n"
            "   mov             %[out3], 0               \n"
            "2:                                          \n"
            "   vldrh.s16       q0, [%[col]], #16        \n"
            "   vldrb.s16        q1, [%[row0]], #8       \n"
            "   vmladava.s16     %[out0], q0, q1         \n"
            "   vldrb.s16        q2, [%[row1]], #8       \n"
            "   vmladava.s16     %[out1], q0, q2         \n"
            "   vldrb.s16        q3, [%[row2]], #8       \n"
            "   vmladava.s16     %[out2], q0, q3         \n"
            "   vldrb.s16        q4, [%[row3]], #8       \n"
            "   vmladava.s16     %[out3], q0, q4         \n"
            "   letp            lr, 2b                   \n"
            "1:                                          \n"
        :
            [col] "+r"(lhs_ptr),
            [row0] "+r"(rhs_ptr_0),
            [row1] "+r"(rhs_ptr_1),
            [row2] "+r"(rhs_ptr_2),
            [row3] "+r"(rhs_ptr_3),
            [out0] "=Te"(result_0),
            [out1] "=Te"(result_1),
            [out2] "=Te"(result_2),
            [out3] "=Te"(result_3)
        :
            [cnt] "r"(rhs_cols_fast)
        :
            "q0", "q1", "q2", "q3", "q4", "memory", "r14"
        );
        // lhs_ptr -= 8;  // Fix lhs_ptr by subtracting 8
#else
        int32_t col_cnt = rhs_cols_fast;

        for (int i_col_loop_cnt = 0; i_col_loop_cnt < col_loop_cnt; i_col_loop_cnt++)
        {
            mve_pred16_t pred = vctp16q(col_cnt);
            col_cnt -= 8;

            int16x8_t lhs_input = vldrhq_z_s16(lhs_ptr, pred);

            int16x8_t rhs_input_0 = vldrbq_z_s16(rhs_ptr_0, pred);
            int16x8_t rhs_input_1 = vldrbq_z_s16(rhs_ptr_1, pred);
            int16x8_t rhs_input_2 = vldrbq_z_s16(rhs_ptr_2, pred);
            int16x8_t rhs_input_3 = vldrbq_z_s16(rhs_ptr_3, pred);

            result_0 = vmladavaq_s16(result_0, lhs_input, rhs_input_0);
            result_1 = vmladavaq_s16(result_1, lhs_input, rhs_input_1);
            result_2 = vmladavaq_s16(result_2, lhs_input, rhs_input_2);
            result_3 = vmladavaq_s16(result_3, lhs_input, rhs_input_3);

            lhs_ptr += 8;

            rhs_ptr_0 += 8;
            rhs_ptr_1 += 8;
            rhs_ptr_2 += 8;
            rhs_ptr_3 += 8;
        }
#endif

        // int64_t result_64_0 = 0;
        // int64_t result_64_1 = 0;
        // int64_t result_64_2 = 0;
        // int64_t result_64_3 = 0;

        // if (rhs_cols > MAX_COL_COUNT)
        // {
        //     int rem_cols = rhs_cols - MAX_COL_COUNT;
        //     __ASM volatile(
        //         " .p2align 2                                 \n"
        //         "   wlstp.16        lr, %[cnt], 1f           \n"
        //         "   mov             %[out0], 0               \n"
        //         "   mov             %[out1], 0               \n"
        //         "   mov             %[out2], 0               \n"
        //         "   mov             %[out3], 0               \n"
        //         "   vldrh.s16       q0, [%[col]], #16        \n"
        //         "2:                                          \n"
        //         "   vldrb.s16        q1, [%[row0]], #8       \n"
        //         "   vmlaldava.s16     %[out0], q0, q1         \n"
        //         "   vldrb.s16        q2, [%[row1]], #8       \n"
        //         "   vmlaldava.s16     %[out1], q0, q2         \n"
        //         "   vldrb.s16       q3, [%[row2]], #8       \n"
        //         "   vmlaldava.s16     %[out2], q0, q3         \n"
        //         "   vldrb.s16        q4, [%[row3]], #8       \n"
        //         "   vmlaldava.s16     %[out3], q0, q4         \n"
        //         "   vldrh.s16        q0, [%[col]], #16       \n"
        //         "   letp            lr, 2b                   \n"
        //         "1:                                          \n"
        //     :
        //         [col] "+r"(lhs_ptr),
        //         [row0] "+r"(rhs_ptr_0),
        //         [row1] "+r"(rhs_ptr_1),
        //         [row2] "+r"(rhs_ptr_2),
        //         [row3] "+r"(rhs_ptr_3),
        //         [out0] "=Te"(result_64_0),
        //         [out1] "=Te"(result_64_1),
        //         [out2] "=Te"(result_64_2),
        //         [out3] "=Te"(result_64_3)
        //     :
        //         [cnt] "r"(rem_cols)
        //     :
        //         "q0", "q1", "q2", "q3", "q4", "memory", "r14"
        //     );
        //     lhs_ptr -= 8;  // Fix lhs_ptr by subtracting 8
        // }

        // result_64_0 += result_0;
        // result_64_1 += result_1;
        // result_64_2 += result_2;
        // result_64_3 += result_3;

        // if (bias)
        // {
        //     result_64_0 += *bias++;
        //     result_64_1 += *bias++;
        //     result_64_2 += *bias++;
        //     result_64_3 += *bias++;
        // }


        int64_t result_64_0 = result_0;
        int64_t result_64_1 = result_1;
        int64_t result_64_2 = result_2;
        int64_t result_64_3 = result_3;

        if (rhs_cols > MAX_COL_COUNT)
        {

#if defined(USE_CUSTOM_INTRINSIC)
            int rem_cols = rhs_cols - MAX_COL_COUNT;
            int32_t col_loop_cnt = (rem_cols + 7) / 8;

            // register const int8_t *rhs_ptr_0 asm("r0") = rhs;
            // register const int8_t *rhs_ptr_1 asm("r1") = rhs_ptr_0 + rhs_cols;
            // register const int8_t *rhs_ptr_2 asm("r2") = rhs_ptr_1 + rhs_cols;
            // register const int8_t *rhs_ptr_3 asm("r3") = rhs_ptr_2 + rhs_cols;

            for (int i_col_loop_cnt = 0; i_col_loop_cnt < col_loop_cnt; i_col_loop_cnt++)
            {
                mve_pred16_t pred = vctp16q(rem_cols);
                rem_cols -= 8;

                int16x8_t lhs_input = vldrhq_z_s16(lhs_ptr, pred);

                int16x8_t rhs_input_0 = vldrbq_z_s16(rhs_ptr_0, pred);
                int16x8_t rhs_input_1 = vldrbq_z_s16(rhs_ptr_1, pred);
                int16x8_t rhs_input_2 = vldrbq_z_s16(rhs_ptr_2, pred);
                int16x8_t rhs_input_3 = vldrbq_z_s16(rhs_ptr_3, pred);

                result_64_0 = vmlaldavaq_s16(result_64_0, lhs_input, rhs_input_0);
                result_64_1 = vmlaldavaq_s16(result_64_1, lhs_input, rhs_input_1);
                result_64_2 = vmlaldavaq_s16(result_64_2, lhs_input, rhs_input_2);
                result_64_3 = vmlaldavaq_s16(result_64_3, lhs_input, rhs_input_3);

                lhs_ptr += 8;

                rhs_ptr_0 += 8;
                rhs_ptr_1 += 8;
                rhs_ptr_2 += 8;
                rhs_ptr_3 += 8;
            }
#else
            // ns_lp_printf("IDDQD!!!!!!\n");
            for (int i_rhs_cols = MAX_COL_COUNT; i_rhs_cols < rhs_cols; i_rhs_cols++)
            {
                const int16_t lhs_temp = *lhs_ptr++;

                result_64_0 += *rhs_ptr_0++ * lhs_temp;
                result_64_1 += *rhs_ptr_1++ * lhs_temp;
                result_64_2 += *rhs_ptr_2++ * lhs_temp;
                result_64_3 += *rhs_ptr_3++ * lhs_temp;
            }
#endif
        }

        if (bias)
        {
            result_64_0 += *bias++;
            result_64_1 += *bias++;
            result_64_2 += *bias++;
            result_64_3 += *bias++;
        }

        int32_t tmp;
        tmp = arm_nn_requantize_s64(result_64_0, dst_multiplier, dst_shift);
        tmp = MAX(tmp, activation_min);
        tmp = MIN(tmp, activation_max);
        *dst++ = (int16_t)tmp;

        tmp = 0;
        tmp = arm_nn_requantize_s64(result_64_1, dst_multiplier, dst_shift);
        tmp = MAX(tmp, activation_min);
        tmp = MIN(tmp, activation_max);
        *dst++ = (int16_t)tmp;

        tmp = 0;
        tmp = arm_nn_requantize_s64(result_64_2, dst_multiplier, dst_shift);
        tmp = MAX(tmp, activation_min);
        tmp = MIN(tmp, activation_max);
        *dst++ = (int16_t)tmp;

        tmp = 0;
        tmp = arm_nn_requantize_s64(result_64_3, dst_multiplier, dst_shift);
        tmp = MAX(tmp, activation_min);
        tmp = MIN(tmp, activation_max);
        *dst++ = (int16_t)tmp;
        rhs += 4 * rhs_cols;
     }

     for (int8_t rows_left = rhs_rows & 0x3; rows_left > 0; rows_left--)
     {
         int32_t result = 0;


         const int16_t *lhs_ptr = lhs;
         const int8_t *rhs_ptr = rhs;
         rhs += rhs_cols;

#if defined(USE_CUSTOM_INTRINSIC)
         __ASM volatile(
             " .p2align 2                                 \n"
             "   wlstp.16        lr, %[cnt], 1f           \n"
             "   mov             %[out0], 0               \n"
             "2:                                          \n"
             "   vldrh.s16       q0, [%[col]], #16        \n"
             "   vldrb.s16        q1, [%[row0]], #8       \n"
             "   vmladava.s16     %[out0], q0, q1         \n"
             "   letp            lr, 2b                   \n"
             "1:                                          \n"
         :
             [col] "+r"(lhs_ptr),
             [row0] "+r"(rhs_ptr),
             [out0] "=Te"(result)
         :
             [cnt] "r"(rhs_cols_fast)
         :
             "q0", "q1", "q2", "q3", "q4", "memory", "r14"
         );
        //  lhs_ptr -= 8;  // Fix lhs_ptr by subtracting 8
#else
         col_loop_cnt = (rhs_cols_fast + 7) / 8;
         int32_t col_cnt = (int32_t)rhs_cols_fast;

         for (int i_col_loop_cnt = 0; i_col_loop_cnt < col_loop_cnt; i_col_loop_cnt++)
         {
             mve_pred16_t pred = vctp16q(col_cnt);
             col_cnt -= 8;

             int16x8_t lhs_input = vldrhq_z_s16(lhs_ptr, pred);
             int16x8_t rhs_input = vldrbq_z_s16(rhs_ptr, pred);

             result = vmladavaq_p_s16(result, lhs_input, rhs_input, pred);

             lhs_ptr += 8;
             rhs_ptr += 8;
         }
#endif

         int64_t result_64 = result;

         if (bias)
         {
             result_64 += *bias++;
         }

         if (rhs_cols > MAX_COL_COUNT)
         {

#if defined(USE_CUSTOM_INTRINSIC)
            int rem_cols = rhs_cols - MAX_COL_COUNT;
            int32_t col_loop_cnt = (rem_cols + 7) / 8;

            for (int i_col_loop_cnt = 0; i_col_loop_cnt < col_loop_cnt; i_col_loop_cnt++)
            {
                mve_pred16_t pred = vctp16q(rem_cols);
                rem_cols -= 8;

                int16x8_t lhs_input = vldrhq_z_s16(lhs_ptr, pred);

                int16x8_t rhs_input_0 = vldrbq_z_s16(rhs_ptr, pred);

                result_64 = vmlaldavaq_s16(result_64, lhs_input, rhs_input_0);

                lhs_ptr += 8;
                rhs_ptr += 8;
            }
#else

             for (int i_rhs_cols = MAX_COL_COUNT; i_rhs_cols < rhs_cols; i_rhs_cols++)
             {
                 const int16_t lhs_temp = *lhs_ptr++;

                 result_64 += *rhs_ptr++ * lhs_temp;
             }
#endif
         }

         int32_t tmp = 0;
         tmp = arm_nn_requantize_s64(result_64, dst_multiplier, dst_shift);
         tmp = MAX(tmp, activation_min);
         tmp = MIN(tmp, activation_max);
         *dst++ = (int16_t)tmp;

     }

    //  ns_lp_printf("IDDQD!!!!!!\n");
     #else // ARM_MATH_MVEI

     const int32_t row_loop_cnt = rhs_rows / 2;

     for (int32_t i = 0; i < row_loop_cnt; i++)
     {

         int64_t acc_64_0 = 0;
         int64_t acc_64_1 = 0;
         int32_t acc_0 = 0;
         int32_t acc_1 = 0;

         const int32_t col_loop_cnt = rhs_cols_fast / 4;

         const int16_t *lhs_vec = lhs;
         const int8_t *rhs_0 = rhs;
         const int8_t *rhs_1 = rhs + rhs_cols;
         rhs += 2 * rhs_cols;

         for (int j = col_loop_cnt; j != 0; j--)
         {
             int32_t ker_0, ker_1, vec_part_0, vec_part_1;

             vec_part_0 = arm_nn_read_q15x2_ia(&lhs_vec);
             vec_part_1 = arm_nn_read_q15x2_ia(&lhs_vec);

             rhs_0 = read_and_pad(rhs_0, &ker_0, &ker_1);

             acc_0 = SMLAD(ker_0, vec_part_0, acc_0);
             acc_0 = SMLAD(ker_1, vec_part_1, acc_0);

             rhs_1 = read_and_pad(rhs_1, &ker_0, &ker_1);

             acc_1 = SMLAD(ker_0, vec_part_0, acc_1);
             acc_1 = SMLAD(ker_1, vec_part_1, acc_1);
         }

         acc_64_0 += acc_0;
         acc_64_1 += acc_1;

         for (int k = col_loop_cnt * 4; k < rhs_cols; k++)
         {
             const int32_t lhs_temp = (*lhs_vec);
             lhs_vec++;
             acc_64_0 += lhs_temp * (*rhs_0);
             rhs_0++;
             acc_64_1 += lhs_temp * (*rhs_1);
             rhs_1++;
         }

         if (bias)
         {
             acc_64_0 += *bias++;
             acc_64_1 += *bias++;
         }
         int32_t tmp;

         tmp = arm_nn_requantize_s64(acc_64_0, dst_multiplier, dst_shift);
         tmp = MAX(tmp, activation_min);
         tmp = MIN(tmp, activation_max);
         *dst++ = (int16_t)tmp;

         tmp = arm_nn_requantize_s64(acc_64_1, dst_multiplier, dst_shift);
         tmp = MAX(tmp, activation_min);
         tmp = MIN(tmp, activation_max);
         *dst++ = (int16_t)tmp;
     }

     if (rhs_rows & 0x1)
     {
         int64_t acc_64_0 = 0;
         int32_t acc_0 = 0;
         const int32_t col_loop_cnt = rhs_cols_fast / 4;

         const int16_t *lhs_vec = lhs;
         const int8_t *rhs_0 = rhs;

         for (int i = col_loop_cnt; i != 0; i--)
         {
             int32_t ker_0, ker_1, vec;
             rhs_0 = read_and_pad(rhs_0, &ker_0, &ker_1);

             vec = arm_nn_read_q15x2_ia(&lhs_vec);
             acc_0 = SMLAD(ker_0, vec, acc_0);

             vec = arm_nn_read_q15x2_ia(&lhs_vec);
             acc_0 = SMLAD(ker_1, vec, acc_0);
         }

         acc_64_0 += acc_0;

         for (int j = col_loop_cnt * 4; j < rhs_cols; j++)
         {
             const int32_t lhs_temp = (*lhs_vec);
             lhs_vec++;
             acc_64_0 += lhs_temp * (*rhs_0);
             rhs_0++;
         }

         if (bias)
         {
             acc_64_0 += *bias++;
         }
         int32_t tmp;
         tmp = arm_nn_requantize_s64(acc_64_0, dst_multiplier, dst_shift);
         tmp = MAX(tmp, activation_min);
         tmp = MIN(tmp, activation_max);
         *dst++ = (int16_t)tmp;
     }

     #endif // ARM_MATH_MVEI
 #else      // ARM_MATH_DSP
     for (int i_row_loop_cnt = 0; i_row_loop_cnt < rhs_rows; i_row_loop_cnt++)
     {
         const int16_t *lhs_ptr = lhs;
         const int8_t *rhs_ptr_0 = &rhs[0];

         int64_t result = 0;

         for (int32_t rhs_cols_idx = 0; rhs_cols_idx < rhs_cols; ++rhs_cols_idx)
         {
             const int64_t rhs_value0 = (int8_t)*rhs_ptr_0;
             const int64_t lhs_value = *lhs_ptr;

             result += lhs_value * rhs_value0;

             ++rhs_ptr_0;
             ++lhs_ptr;
         }

         if (bias)
         {
             result += *bias++;
         }
         // Quantize down
         result = arm_nn_requantize_s64(result, dst_multiplier, dst_shift);

         // Clamp the result
         result = MAX(result, activation_min);
         result = MIN(result, activation_max);

         *dst++ = (int16_t)result;
         rhs += rhs_cols;
     }
 #endif     // ARM_MATH_DSP

     return ARM_CMSIS_NN_SUCCESS;
 }

 /**
  * @} end of Doxygen group
  */


uint32_t
test_fc_16x8_init() {

    for (int i = 0; i < INPUT_LEN * OUTPUT_LEN; i++) {
        fc_weights[i] = (int8_t)(i % 128);
    }

    return 0;
}

uint32_t
test_fc_16x8_run(int16_t *input, int16_t *output) {

    arm_cmsis_nn_status status;
    uint32_t ticUs, tocUs;
    cmsis_nn_context ctx;
    ctx.buf = ctx_buffer;
    ctx.size = sizeof(ctx_buffer);

    const int16_t *in_ptr = input;
    int16_t *out_ptr = act_buffer0;

    ticUs = ns_us_ticker_read(&timerCfg);

    status = test_arm_nn_vec_mat_mult_t_s16(
        in_ptr,
        fc_weights,
        fc_bias,
        out_ptr,
        REDUCE_MULTIPLIER(quant_params_fc.multiplier),
        quant_params_fc.shift,
        fc_filter_dims.n, /* col_dim or accum_depth */
        fc_output_dims.c, /* row_dim or output_depth */
        fc_params_linear.activation.min,
        fc_params_linear.activation.max
    );

    if (status != ARM_CMSIS_NN_SUCCESS)
        return status;

    tocUs = ns_us_ticker_read(&timerCfg);

    ns_lp_printf("Time: %d us\n", tocUs - ticUs);

    // Copy output to the output buffer
    memcpy(output, out_ptr, OUTPUT_LEN * sizeof(int16_t));

    return status;

}
