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
#include "main.h"
#include "test_library.h"


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

int main(void)
{
    hardware_init();
    ns_timer_init(&timerCfg);

    while (1) {
        if (!test_library_done()) {
            test_library_step(0);  
        } else {
            ns_lp_printf("[CMSIS-NN] All tests done. Idling...\n");
            ns_delay_us(5000000);
        }
        ns_delay_us(20000);
    }
}