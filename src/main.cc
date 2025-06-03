/**
 * @file main.c
 * @author Adam Page (adam.page@ambiq.com)
 * @version 1.0
 * @date 2024-12-25
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <arm_math.h>
#include <arm_mve.h>
// neuralSPOT
#include "ns_ambiqsuite_harness.h"
#include "ns_peripherals_power.h"
// Locals
#include "aot_test_case.h"
// #include "tflm.h"
// #include "model.h"
#include "main.h"


ns_timer_config_t timerCfg = {
    .api = &ns_timer_V1_0_0,
    .timer = NS_TIMER_COUNTER,
    .enableInterrupt = false
};


void
hardware_init(void) {
    ns_core_config_t nsCoreCfg = {
        .api = &ns_core_V1_0_0
    };

    NS_TRY(ns_core_init(&nsCoreCfg), "Core Init failed.\b");
    NS_TRY(ns_power_config(&ns_development_default), "Power Init Failed\n");
    ns_itm_printf_enable();
    ns_interrupt_master_enable();
    // ns_itm_pcsamp_enable();
    ns_set_performance_mode(NS_MAXIMUM_PERF);
}

// NS_PUT_IN_TCM int8_t model_ad_input[640] = {0};
// NS_PUT_IN_TCM int8_t model_ad_output[640] = {0};

// NS_PUT_IN_TCM int8_t aot_input[kws_input_len] = {0};
// NS_PUT_IN_TCM int8_t aot_output[kws_output_len] = {0};

// int16_t test_fc_16x8_input[640] = {0};
// int16_t test_fc_16x8_output[640] = {0};

// #define TEST_LEN (5000)
// static const int8_t test_src[TEST_LEN] = {1};
// static NS_PUT_IN_TCM int8_t test_dst[TEST_LEN] = {0};

// static NS_PUT_IN_TCM int8_t test_fc_s8_input[960] = {0};
// static NS_PUT_IN_TCM int8_t test_fc_s8_output[240] = {0};
// AM_SHARED_RW const



// static void test_stimulus_callback(
//     int32_t op,
//     vww_operator_state_e state,
//     int32_t status,
//     void *user_data
// ) {
//     uint32_t glb_ticUs = 0;
//     if (state == vww_model_state_started) {
//         ns_timer_clear(&timerCfg);
//     } else
//     if (state == vww_model_state_finished) {
//         glb_ticUs = ns_us_ticker_read(&timerCfg);
//         ns_lp_printf("%d,%d\n", op, glb_ticUs);
//     }
// }

// vww_model_context_t aot_model_context = {
//     .callback = NULL,
//     .user_data = NULL
// };

// static AM_SHARED_RW int8_t test_stimulus_input[vww_input_len];
// static AM_SHARED_RW int8_t test_stimulus_output[vww_output_len];

int
main(void)
{
    bool result = false;
    uint32_t status = 0;
    uint32_t ticUs = 0, tocUs = 0, tacUs = 0;
    hardware_init();
    NS_TRY(aot_test_case_init(), "AOT Test Case Init Failed\n");
    // NS_TRY(tflm_init(), "TFLM Init Failed\n");
    // NS_TRY(model_init(), "MODEL Init Failed\n");
    // NS_TRY(strm_gg_model_init(&aot_model_context), "AOT Model Init Failed\n");
    ns_timer_init(&timerCfg);

    while (1) {
        ns_printf("Running Inference...\n");

        ns_timer_clear(&timerCfg);
        ticUs = ns_us_ticker_read(&timerCfg);
        status = aot_test_case_run();
        tocUs = ns_us_ticker_read(&timerCfg);
        ns_lp_printf("AOT Test Case Run: %d us (%d)\n", tocUs - ticUs, status);

        // for (int i = 0; i < vww_input_len; i++) {
        //     test_stimulus_input[i] += (int8_t)(i % 16);
        // }
        // status = vww_model_run(
        //     &aot_model_context,
        //     test_stimulus_input,
        //     test_stimulus_output
        // );
        // tocUs = ns_us_ticker_read(&timerCfg);
        // ns_lp_printf("[MODEL AD] Inference time: %d us (%d)\n", tocUs - ticUs, status);
        // status = kws_test_case_run();

        // for (int i = 0; i < var_input_len; i++) {
        //     aot_input[i] += (int8_t)(i % 16);
        //     ns_lp_printf("Input[%d]: %d\n", i, aot_input[i]);
        // }
        // ticUs = ns_us_ticker_read(&timerCfg);
        // status = tpc_model_run(aot_input, aot_output);
        // tocUs = ns_us_ticker_read(&timerCfg);
        // ns_lp_printf("[MODEL AOT] Inference time: %d us (%d)\n", tocUs - ticUs, status);
        // for (int i = 0; i < var_output_len; i++) {
        //     ns_lp_printf("Output[%d]: %d\n", i, tpc_output[i]);
        // }

        // ticUs = ns_us_ticker_read(&timerCfg);
        // status = model_inference();
        // tocUs = ns_us_ticker_read(&timerCfg);
        // ns_lp_printf("Model Inference: %d us (%d)\n", tocUs - ticUs, status);

        // for (int i = 0; i < TEST_LEN; i++) {
        //     test_src[i] += (int8_t)(i % 16);
        // }

        // for (int i = 0; i < 960; i++) {
        //     // model_ad_input[i] += (int8_t)(i % 16);
        //     // test_fc_16x8_input[i] += (int16_t)(i % 16);
        //     test_fc_s8_input[i] += (int16_t)(i % 16);
        // }
        // arm_memcpy_s8(test_dst, test_src, TEST_LEN);
        // tocUs = ns_us_ticker_read(&timerCfg);
        // arm_memcpy_s8(test_dst, test_src, TEST_LEN);
        // tacUs = ns_us_ticker_read(&timerCfg);
        // ns_lp_printf("MEM COPY: %d us %d us\n", tocUs - ticUs, tacUs - tocUs);

        // status = test_fc_16x8_run(test_fc_16x8_input, test_fc_16x8_output);
        // status = model_ad_run(model_ad_input, model_ad_output);
        // status = test_fc_s8_run(test_fc_s8_input, test_fc_s8_output);
        ns_delay_us(5000000);
    };
}
