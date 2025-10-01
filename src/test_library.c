#include <stdint.h>
#include <stdlib.h>  
#include <string.h>   
#include "ns_ambiqsuite_harness.h"
#include "test_library.h"


extern ns_timer_config_t timerCfg;

volatile int __unity_failures = 0;

#ifndef TEST_BATCH_SIZE
#define TEST_BATCH_SIZE 5   
#endif
#ifndef TEST_YIELD_US
#define TEST_YIELD_US  20000 
#endif


#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_add_s16/test_arm_add_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_add_s8/test_arm_add_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_avgpool_s16/test_arm_avgpool_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_avgpool_s8/test_arm_avgpool_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_batch_matmul_s16/test_arm_batch_matmul_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_batch_matmul_s8/test_arm_batch_matmul_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_convolve_1_x_n_s8/test_arm_convolve_1_x_n_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_convolve_1x1_s4_fast/test_arm_convolve_1x1_s4_fast.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_convolve_1x1_s8_fast/test_arm_convolve_1x1_s8_fast.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_convolve_s16/test_arm_convolve_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_convolve_s4/test_arm_convolve_s4.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_convolve_s8/test_arm_convolve_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_depthwise_conv_3x3_s8/test_arm_depthwise_conv_3x3_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_depthwise_conv_fast_s16/test_arm_depthwise_conv_fast_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_depthwise_conv_s16/test_arm_depthwise_conv_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_depthwise_conv_s4/test_arm_depthwise_conv_s4.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_depthwise_conv_s4_opt/test_arm_depthwise_conv_s4_opt.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_depthwise_conv_s8/test_arm_depthwise_conv_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_depthwise_conv_s8_opt/test_arm_depthwise_conv_s8_opt.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_dequantize_s16_f32/test_arm_dequantize_s16_f32.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_dequantize_s8_f32/test_arm_dequantize_s8_f32.c"
#define in_out_buf_main ds_cnn_l_in_out_buf_main
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_ds_cnn_l_s8/test_arm_ds_cnn_l_s8.c"
#undef in_out_buf_main

#define in_out_buf_main ds_cnn_s_in_out_buf_main
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_ds_cnn_s_s8/test_arm_ds_cnn_s_s8.c"
#undef in_out_buf_main
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_elementwise_add_s16/test_arm_elementwise_add_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_elementwise_add_s8/test_arm_elementwise_add_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_elementwise_mul_s16/test_arm_elementwise_mul_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_elementwise_mul_s8/test_arm_elementwise_mul_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_fully_connected_s16/test_arm_fully_connected_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_fully_connected_s4/test_arm_fully_connected_s4.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_fully_connected_s8/test_arm_fully_connected_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_grouped_convolve_s8/test_arm_grouped_convolve_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_leaky_relu_s8/test_arm_leaky_relu_s8.c"
#define buffer1 lstm_s16_buffer1
#define buffer2 lstm_s16_buffer2
#define buffer3 lstm_s16_buffer3
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_lstm_unidirectional_s16/test_arm_lstm_unidirectional_s16.c"
#undef buffer1
#undef buffer2
#undef buffer3

#define buffer1 lstm_s8_buffer1
#define buffer2 lstm_s8_buffer2
#define buffer3 lstm_s8_buffer3
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_lstm_unidirectional_s8/test_arm_lstm_unidirectional_s8.c"
#undef buffer1
#undef buffer2
#undef buffer3
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_max_pool_s16/test_arm_max_pool_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_max_pool_s8/test_arm_max_pool_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_maximum_minimum_s16/test_arm_maximum_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_maximum_minimum_s16/test_arm_minimum_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_maximum_minimum_s8/test_arm_maximum_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_maximum_minimum_s8/test_arm_minimum_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_mean_s16/test_arm_mean_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_mean_s8/test_arm_mean_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_mul_s16/test_arm_mul_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_mul_s8/test_arm_mul_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_pad_s16/test_arm_pad_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_pad_s8/test_arm_pad_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_quantize_f32_s16/test_arm_quantize_f32_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_quantize_f32_s8/test_arm_quantize_f32_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_quantize_s16_s16/test_arm_quantize_s16_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_quantize_s8_s8/test_arm_quantize_s8_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_reduce_max_s16/test_arm_reduce_max_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_reduce_max_s8/test_arm_reduce_max_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_relu_s8/test_arm_relu_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_reverse_transpose_conv_s8/test_arm_reverse_transpose_conv_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_softmax_s16/test_arm_softmax_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_softmax_s8/test_arm_softmax_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_softmax_s8_s16/test_arm_softmax_s8_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_strided_slice_s8/test_arm_strided_slice_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_svdf_s8/test_arm_svdf_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_svdf_state_s16_s8/test_arm_svdf_state_s16_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_transpose_conv_s8/test_arm_transpose_conv_s8.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_transpose_s16/test_arm_transpose_s16.c"
#include "../modules/ns-cmsis-nn/Tests/UnitTest/TestCases/test_arm_transpose_s8/test_arm_transpose_s8.c"

typedef void (*test_fn_t)(void);


#define X(fn) extern void fn(void);
TEST_LIST
#undef X

#define X(fn) fn,
static void (*const kTests[])(void) = { TEST_LIST };
#undef X

#define X(fn) #fn,
static const char *const kNames[] = { TEST_LIST };
#undef X

static const size_t kNumTests = sizeof(kTests) / sizeof(kTests[0]);
static size_t g_cursor = 0;  
static int g_passed = 0; 

static inline uint32_t tic_us(void) { ns_timer_clear(&timerCfg); return ns_us_ticker_read(&timerCfg); }
static inline uint32_t toc_us(uint32_t t0) { return ns_us_ticker_read(&timerCfg) - t0; }

static void run_one(size_t idx) {
    // __unity_failures = 0;
    // uint32_t t0 = tic_us();
    kTests[idx]();         
    // uint32_t us = toc_us(t0);

    // const int ok = (__unity_failures == 0);
    // if (ok) g_passed++;

    // ns_lp_printf("[CMSIS-NN][%s] %s (%lu us)\n",
    //                 kNames[idx], ok ? "PASS" : "FAIL", (unsigned long)us);
}

void test_library_step(unsigned budget) {
    if (g_cursor == 0) {
        ns_lp_printf("\n[CMSIS-NN] %u total tests queued\n", (unsigned)kNumTests);
    }
    if (budget == 0) budget = TEST_BATCH_SIZE;

    while (budget-- && g_cursor < kNumTests) {
        run_one(g_cursor++);
        ns_delay_us(TEST_YIELD_US);
    }

    if (g_cursor == kNumTests) {
        ns_lp_printf("[CMSIS-NN] Summary: %s (%d/%u passed)\n\n",
                    (g_passed == (int)kNumTests) ? "PASS" : "FAIL",
                    g_passed, (unsigned)kNumTests);
    }
}

int test_library_done(void) { return g_cursor >= kNumTests; }

void test_library_reset(void) {
    g_cursor = 0;
    g_passed = 0;
}

void test_library(void) {
    while (!test_library_done()) {
        test_library_step(TEST_BATCH_SIZE);
    }
}  