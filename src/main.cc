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
#include "main.h"

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

int
main(void)
{
    uint32_t status;
    hardware_init();
    NS_TRY(tflm_init(), "TFLM Init Failed\n");
    NS_TRY(model_init(), "MODEL Init Failed\n");

    while (1) {
        ns_printf("Running Inference...\n");
        status = model_inference();
        ns_printf("Inference Status: %d\n", status);
        ns_delay_us(500000);
    };
}
