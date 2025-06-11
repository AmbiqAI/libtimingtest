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
#include "aot_model.h"
#include "aot_test_case.h"
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


static int8_t aot_input_0[aot_input_0_size];
static int8_t aot_output_0[aot_output_0_size];
// static int8_t aot_output_1[aot_output_1_size];
// static int8_t aot_output_2[aot_output_2_size];
// static int8_t aot_output_3[aot_output_3_size];

// static void test_stimulus_callback(
//     int32_t op,
//     aot_operator_state_e state,
//     int32_t status,
//     void *user_data
// ) {
//     uint32_t glb_ticUs = 0;
//     if (state == aot_model_state_started) {
//         // ns_timer_clear(&timerCfg);
//     } else
//     if (state == aot_model_state_finished) {
//         glb_ticUs = ns_us_ticker_read(&timerCfg);
//         // ns_lp_printf("%d,%d\n", op, glb_ticUs);
//     }
// }

aot_model_context_t aot_model_context = {
    .input_data = {
        aot_input_0
    },
    .input_len = {
        aot_input_0_size
    },
    .output_data = {
        aot_output_0,
        // aot_output_1,
        // aot_output_2,
        // aot_output_3
    },
    .output_len = {
        aot_output_0_size,
        // aot_output_1_size,
        // aot_output_2_size,
        // aot_output_3_size
    },
    .callback = NULL, // Uncomment to use the callback
    // .callback = test_stimulus_callback,
    .user_data = NULL
};


int
main(void)
{
    uint32_t status = 0;
    uint32_t ticUs = 0, tocUs = 0;
    hardware_init();
    NS_TRY(aot_model_init(&aot_model_context), "AOT Model Init Failed\n");
    NS_TRY(aot_test_case_init(), "AOT Test Case Init Failed\n");
    ns_timer_init(&timerCfg);

    while (1) {

        status = aot_test_case_run();
        ns_lp_printf("HeliosAOT Test Case Run: %d\n", status);

        ns_printf("Running Inference...\n");
        ns_timer_clear(&timerCfg);
        ticUs = ns_us_ticker_read(&timerCfg);
        status = aot_model_run(&aot_model_context);
        tocUs = ns_us_ticker_read(&timerCfg);
        ns_lp_printf("HeliosAOT Inference Run: %d us (%d)\n", tocUs - ticUs, status);

        ns_delay_us(5000000);
    };
}
