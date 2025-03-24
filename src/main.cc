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
// #include "tflm.h"
// #include "model.h"
// #include "model_ad.h"
// #include "test_fc_16x8.h"
// #include "arm_nnsupportfunctions.h"  // CMSIS-NN API
// #include "test_fc_s8.h"
#include "algorithm.h"
#include "main.h"

#define SAMPLE_RATE      25.0f
#define SIGNAL_FREQUENCY  3.4f
#define SIGNAL_LEVEL      1.0f
#define SIGNAL_PHASE      0.4f
#define NOISE_LEVEL      11.4f
#define FIFO_LEN         1
#define INPUT_INTERVAL_MS 40

enum mode {
    MODE_OFF,  // Deep sleep
    MODE_ON,   // Continuous operation
    MODE_1HZ,  // 1 Hz interrupts with simulated FIFO length of 25
    MODE_5HZ,  // 5 Hz interrupts with simulated FIFO length of 5
    MODE_25HZ, // 25 Hz interrupts without simulated FIFO
    NUMBER_OF_MODES
};

// Algorithm instances
static struct algorithm algo;
static struct input_generator gen;
static NS_PUT_IN_TCM float16_t tempBuffer[3 * 1024 + 512];
static volatile int algorithm_run_count = 0;

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
    ns_set_performance_mode(NS_MINIMUM_PERF);
}

static bool
generate_input_and_run_algorithm(void) {
    struct algorithm_input input;

    float16_t clean = generate_sine(&gen);
    for (int i = 0; i < MAX_INPUT_CHANNELS; i++) {
        input.channels[i] = clean + generate_white_noise(NOISE_LEVEL);
    }
    input.num_channels = MAX_INPUT_CHANNELS;
    input.sample_interval = 1.0f / SAMPLE_RATE;

    float16_t output;
    return algorithm(&algo, &input, tempBuffer, &output);
}

void
init_algorithm_and_generator(void) {
    algorithm_init(&algo);
    input_generator_init(&gen, SIGNAL_FREQUENCY, SIGNAL_LEVEL, SIGNAL_PHASE, SAMPLE_RATE);
}

// int8_t model_ad_input[640] = {0};
// int8_t model_ad_output[640] = {0};

// int16_t test_fc_16x8_input[640] = {0};
// int16_t test_fc_16x8_output[640] = {0};

// #define TEST_LEN (5000)
// static const int8_t test_src[TEST_LEN] = {1};
// static NS_PUT_IN_TCM int8_t test_dst[TEST_LEN] = {0};

// static NS_PUT_IN_TCM int8_t test_fc_s8_input[960] = {0};
// static NS_PUT_IN_TCM int8_t test_fc_s8_output[240] = {0};
// AM_SHARED_RW const

int
main(void)
{
    bool result = false;
    uint32_t status = 0;
    uint32_t ticUs = 0, tocUs = 0, tacUs = 0;
    hardware_init();
    init_algorithm_and_generator();
    // NS_TRY(tflm_init(), "TFLM Init Failed\n");
    // NS_TRY(model_init(), "MODEL Init Failed\n");
    // NS_TRY(model_ad_init(), "MODEL AD Init Failed\n");
    // NS_TRY(test_fc_16x8_init(), "Test FC 16x8 Init Failed\n");
    // NS_TRY(test_fc_s8_init(), "Test FC S8 Init Failed\n");
    ns_timer_init(&timerCfg);

    while (1)
    {
        ns_delay_us(INPUT_INTERVAL_MS * 1000 * 1);
        ns_timer_clear(&timerCfg);
        ticUs = ns_us_ticker_read(&timerCfg);
        result = generate_input_and_run_algorithm();
        tocUs = ns_us_ticker_read(&timerCfg);
        if (result) {
            algorithm_run_count++;
            ns_lp_printf("Algorithm time: %d us\n", tocUs - ticUs);
        } else {
            ns_lp_printf(".");
        }
    }

    // SCB_EnableICache();
    // SCB_DisableDCache();
    while (1) {
        // ns_printf("Running Inference...\n");
        // status = model_inference();

        // for (int i = 0; i < TEST_LEN; i++) {
        //     test_src[i] += (int8_t)(i % 16);
        // }

        // for (int i = 0; i < 960; i++) {
        //     // model_ad_input[i] += (int8_t)(i % 16);
        //     // test_fc_16x8_input[i] += (int16_t)(i % 16);
        //     test_fc_s8_input[i] += (int16_t)(i % 16);
        // }
        // ns_timer_clear(&timerCfg);
        // SCB_InvalidateDCache();
        // ticUs = ns_us_ticker_read(&timerCfg);
        // arm_memcpy_s8(test_dst, test_src, TEST_LEN);
        // tocUs = ns_us_ticker_read(&timerCfg);
        // arm_memcpy_s8(test_dst, test_src, TEST_LEN);
        // tacUs = ns_us_ticker_read(&timerCfg);
        // ns_lp_printf("MEM COPY: %d us %d us\n", tocUs - ticUs, tacUs - tocUs);

        // status = test_fc_16x8_run(test_fc_16x8_input, test_fc_16x8_output);
        // status = model_ad_run(model_ad_input, model_ad_output);
        // status = test_fc_s8_run(test_fc_s8_input, test_fc_s8_output);
        // ns_lp_printf("[MODEL AD] Inference time: %d us %d (%d)\n", tocUs - ticUs, tacUs - tocUs, status);
        ns_delay_us(1000000);
        // ns_lp_printf("status=%d\n", status);
    };
}
