/**
 * @file main.c
 * @author Adam Page (adam.page@ambiq.com)
 * @version 1.0
 * @date 2024-12-25
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <arm_mve.h>
// neuralSPOT
#include "ns_ambiqsuite_harness.h"
#include "ns_peripherals_power.h"
// Locals
#include "tflm.h"
#include "model.h"
#include "model_ad.h"
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
}

int8_t model_ad_input[640] = {0};
int8_t model_ad_output[640] = {0};


int
main(void)
{
    uint32_t status;
    uint32_t ticUs = 0, tocUs = 0;
    hardware_init();
    NS_TRY(tflm_init(), "TFLM Init Failed\n");
    NS_TRY(model_init(), "MODEL Init Failed\n");
    NS_TRY(model_ad_init(), "MODEL AD Init Failed\n");
    ns_timer_init(&timerCfg);

    while (1) {
        ns_printf("Running Inference...\n");
        status = model_inference();

        for (int i = 0; i < 640; i++) {
            model_ad_input[i] += (int8_t)(i % 16);
        }
        ns_timer_clear(&timerCfg);
        ticUs = ns_us_ticker_read(&timerCfg);
        status = model_ad_run(model_ad_input, model_ad_output);
        tocUs = ns_us_ticker_read(&timerCfg);
        ns_lp_printf("[MODEL AD] Inference time: %d us (%d)\n", tocUs - ticUs, status);
        ns_delay_us(2000000);
        ns_lp_printf("%d\n", model_ad_output[0]);
    };
}
