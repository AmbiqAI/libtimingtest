#include <stdint.h>
#include "ns_ambiqsuite_harness.h"
#include "arm_nnfunctions.h"
#include "ns_perf_profile.h"
#include "ns_pmu_utils.h"
#include <stddef.h> 

void *ns_malloc(size_t size);
void  ns_free(void *ptr);


void *__wrap_malloc(size_t size) {
    return ns_malloc(size);
}

void __wrap_free(void *ptr) {
    ns_free(ptr);
}

extern ns_timer_config_t timerCfg;

// PMU and DWT counter configurations
static ns_pmu_config_t pmuCfg;
static ns_perf_counters_t dwtStart, dwtEnd, dwtDelta;
static ns_pmu_counters_t pmuStart, pmuEnd, pmuDelta;
static bool pmu_initialized = false;
static bool dwt_initialized = false;

static inline uint32_t tic_us(void)
{
  ns_timer_clear(&timerCfg);
  return ns_us_ticker_read(&timerCfg);
}

static inline uint32_t toc_us(uint32_t t0) { return ns_us_ticker_read(&timerCfg) - t0; }

static inline void log_kernel(const char *name, uint32_t us)
{
  ns_lp_printf("[KERNEL][%s] %lu\n", name, (unsigned long)us);
}

// Initialize DWT profiler if not already done
static void init_dwt_if_needed(void)
{
  if (!dwt_initialized) {
    // Initialize and start the DWT profiler
    ns_init_perf_profiler();
    ns_start_perf_profiler();
    
    // Give DWT some time to start counting
    ns_delay_us(1000);
    
    dwt_initialized = true;
  }
}

// Initialize PMU counters if not already done
static void init_pmu_if_needed(void)
{
  if (!pmu_initialized) {
    // Initialize PMU with specific performance events
    pmuCfg.api = &ns_pmu_current_version;
    
    // Configure PMU events as requested
    pmuCfg.events[0].enabled = true;
    pmuCfg.events[0].eventId = 0x0200; // ARM_PMU_MVE_INST_RETIRED
    pmuCfg.events[0].counterSize = NS_PMU_EVENT_COUNTER_SIZE_32;
    
    pmuCfg.events[1].enabled = true;
    pmuCfg.events[1].eventId = 0x0228; // ARM_PMU_MVE_INT_MAC_RETIRED
    pmuCfg.events[1].counterSize = NS_PMU_EVENT_COUNTER_SIZE_32;
    
    pmuCfg.events[2].enabled = true;
    pmuCfg.events[2].eventId = 0x0008; // ARM_PMU_INST_RETIRED
    pmuCfg.events[2].counterSize = NS_PMU_EVENT_COUNTER_SIZE_32;
    
    pmuCfg.events[3].enabled = true;
    pmuCfg.events[3].eventId = 0x001D; // ARM_PMU_BUS_CYCLES
    pmuCfg.events[3].counterSize = NS_PMU_EVENT_COUNTER_SIZE_32;
    
    pmuCfg.events[4].enabled = true;
    pmuCfg.events[4].eventId = 0x0011; // ARM_PMU_CPU_CYCLES
    pmuCfg.events[4].counterSize = NS_PMU_EVENT_COUNTER_SIZE_32;
    
    pmuCfg.events[5].enabled = true;
    pmuCfg.events[5].eventId = 0x0003; // ARM_PMU_L1D_CACHE_REFILL
    pmuCfg.events[5].counterSize = NS_PMU_EVENT_COUNTER_SIZE_32;
    
    pmuCfg.events[6].enabled = true;
    pmuCfg.events[6].eventId = 0x0001; // ARM_PMU_L1I_CACHE_REFILL
    pmuCfg.events[6].counterSize = NS_PMU_EVENT_COUNTER_SIZE_32;
    
    // // Initialize PMU
    // if (ns_pmu_init(&pmuCfg) == NS_STATUS_SUCCESS) {
    //   pmu_initialized = true;
    //   ns_lp_printf("[PMU] Initialized with 7 counters: MVE_INST_RETIRED, MVE_INT_MAC_RETIRED, INST_RETIRED, BUS_CYCLES, CPU_CYCLES, L1D_CACHE_REFILL, L1I_CACHE_REFILL\n");
    // } else {
    //   ns_lp_printf("[PMU] Failed to initialize PMU counters\n");
    // }
  }
}

// Capture PMU and DWT counters before kernel call
static void capture_start_counters(void)
{
  init_dwt_if_needed();
  init_pmu_if_needed();
  
  // Capture DWT counters
  ns_capture_perf_profiler(&dwtStart);
  
  
  // Capture PMU counters if initialized
  if (pmu_initialized) {
    ns_pmu_get_counters(&pmuCfg);
    for (int i = 0; i < 8; i++) {
      if (pmuCfg.events[i].enabled) {
        pmuStart.counterValue[i] = pmuCfg.counter[i].counterValue;
      }
    }
  }
}

// Capture PMU and DWT counters after kernel call and log them
static void capture_end_counters_and_log(const char *name, uint32_t timing_us, arm_cmsis_nn_status status)
{
  // Capture DWT counters
  ns_capture_perf_profiler(&dwtEnd);
  
  
  // Calculate DWT delta
  ns_delta_perf(&dwtStart, &dwtEnd, &dwtDelta);
  
  // Capture PMU counters if initialized
  if (pmu_initialized) {
    ns_pmu_get_counters(&pmuCfg);
    for (int i = 0; i < 8; i++) {
      if (pmuCfg.events[i].enabled) {
        pmuEnd.counterValue[i] = pmuCfg.counter[i].counterValue;
      }
    }
    
    // Calculate PMU delta
    ns_delta_pmu(&pmuStart, &pmuEnd, &pmuDelta);
  }
  
  // Log all counters in one line: kernel_name, time, status, counter1, counter2, etc.
  const char* status_str = (status == ARM_CMSIS_NN_SUCCESS) ? "SUCCESS" : "FAILURE";
  ns_lp_printf("%s, Time=%lu, Status=%s(%d), ", name, (unsigned long)timing_us, status_str, (int)status);
  
  // DWT counters
  ns_lp_printf("DWT_cycles=%lu, DWT_instructions=%lu, DWT_cpi=%lu, DWT_exceptions=%lu, DWT_sleep=%lu, DWT_lsu=%lu, DWT_fold=%lu, ", 
               (unsigned long)(dwtDelta.cyccnt - dwtDelta.cpicnt - dwtDelta.exccnt - dwtDelta.sleepcnt - dwtDelta.lsucnt + dwtDelta.foldcnt),
               (unsigned long)dwtDelta.cpicnt, (unsigned long)dwtDelta.exccnt, 
               (unsigned long)dwtDelta.sleepcnt, (unsigned long)dwtDelta.lsucnt, (unsigned long)dwtDelta.foldcnt);
  
  // PMU counters
  if (pmu_initialized) {
    if (pmuCfg.events[0].enabled) ns_lp_printf("MVE_INST_RETIRED=%lu, ", (unsigned long)pmuDelta.counterValue[0]);
    if (pmuCfg.events[1].enabled) ns_lp_printf("MVE_INT_MAC_RETIRED=%lu, ", (unsigned long)pmuDelta.counterValue[1]);
    if (pmuCfg.events[2].enabled) ns_lp_printf("INST_RETIRED=%lu, ", (unsigned long)pmuDelta.counterValue[2]);
    if (pmuCfg.events[3].enabled) ns_lp_printf("BUS_CYCLES=%lu, ", (unsigned long)pmuDelta.counterValue[3]);
    if (pmuCfg.events[4].enabled) ns_lp_printf("CPU_CYCLES=%lu, ", (unsigned long)pmuDelta.counterValue[4]);
    if (pmuCfg.events[5].enabled) ns_lp_printf("L1D_CACHE_REFILL=%lu, ", (unsigned long)pmuDelta.counterValue[5]);
    if (pmuCfg.events[6].enabled) ns_lp_printf("L1I_CACHE_REFILL=%lu, ", (unsigned long)pmuDelta.counterValue[6]);
  }
  
  ns_lp_printf("\n");
}

// arm_add_s16
arm_cmsis_nn_status __real_arm_add_s16(
    const int16_t *input1_data, const cmsis_nn_dims *input1_dims, const int16_t *input2_data,
    const cmsis_nn_dims *input2_dims, const int32_t input1_offset, const int32_t input1_mult,
    const int32_t input1_shift, const int32_t input2_offset, const int32_t input2_mult, const int32_t input2_shift,
    const int32_t left_shift, int16_t *output_data, const cmsis_nn_dims *output_dims, const int32_t out_offset,
    const int32_t out_mult, const int32_t out_shift, const int32_t out_activation_min,
    const int32_t out_activation_max);

arm_cmsis_nn_status __wrap_arm_add_s16(
    const int16_t *input1_data, const cmsis_nn_dims *input1_dims, const int16_t *input2_data,
    const cmsis_nn_dims *input2_dims, const int32_t input1_offset, const int32_t input1_mult,
    const int32_t input1_shift, const int32_t input2_offset, const int32_t input2_mult, const int32_t input2_shift,
    const int32_t left_shift, int16_t *output_data, const cmsis_nn_dims *output_dims, const int32_t out_offset,
    const int32_t out_mult, const int32_t out_shift, const int32_t out_activation_min, const int32_t out_activation_max)
{
  uint32_t t0 = tic_us();
  capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_add_s16(
      input1_data, input1_dims, input2_data, input2_dims, input1_offset, input1_mult, input1_shift, input2_offset,
      input2_mult, input2_shift, left_shift, output_data, output_dims, out_offset, out_mult, out_shift,
      out_activation_min, out_activation_max);
  capture_end_counters_and_log("arm_add_s16", toc_us(t0), rc);
  return rc;
}

// arm_add_s8
arm_cmsis_nn_status __real_arm_add_s8(
    const int8_t *input1_data, const cmsis_nn_dims *input1_dims, const int8_t *input2_data,
    const cmsis_nn_dims *input2_dims, const int32_t input1_offset, const int32_t input1_mult,
    const int32_t input1_shift, const int32_t input2_offset, const int32_t input2_mult, const int32_t input2_shift,
    const int32_t left_shift, int8_t *output_data, const cmsis_nn_dims *output_dims, const int32_t out_offset,
    const int32_t out_mult, const int32_t out_shift, const int32_t out_activation_min,
    const int32_t out_activation_max);

arm_cmsis_nn_status __wrap_arm_add_s8(
    const int8_t *input1_data, const cmsis_nn_dims *input1_dims, const int8_t *input2_data,
    const cmsis_nn_dims *input2_dims, const int32_t input1_offset, const int32_t input1_mult,
    const int32_t input1_shift, const int32_t input2_offset, const int32_t input2_mult, const int32_t input2_shift,
    const int32_t left_shift, int8_t *output_data, const cmsis_nn_dims *output_dims, const int32_t out_offset,
    const int32_t out_mult, const int32_t out_shift, const int32_t out_activation_min, const int32_t out_activation_max)
{
  uint32_t t0 = tic_us();
  capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_add_s8(
      input1_data, input1_dims, input2_data, input2_dims, input1_offset, input1_mult, input1_shift, input2_offset,
      input2_mult, input2_shift, left_shift, output_data, output_dims, out_offset, out_mult, out_shift,
      out_activation_min, out_activation_max);
  capture_end_counters_and_log("arm_add_s8", toc_us(t0), rc);
  log_kernel("arm_add_s8", toc_us(t0));
  return rc;
}

// arm_avgpool_s16
arm_cmsis_nn_status __real_arm_avgpool_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_pool_params *pool_params, const cmsis_nn_dims *input_dims,
    const int16_t *input_data, const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims,
    int16_t *output_data);

arm_cmsis_nn_status __wrap_arm_avgpool_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_pool_params *pool_params, const cmsis_nn_dims *input_dims,
    const int16_t *input_data, const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims, int16_t *output_data)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc =
      __real_arm_avgpool_s16(ctx, pool_params, input_dims, input_data, filter_dims, output_dims, output_data);
  log_kernel("arm_avgpool_s16", toc_us(t0));
  return rc;
}

// arm_avgpool_s8
arm_cmsis_nn_status __real_arm_avgpool_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_pool_params *pool_params, const cmsis_nn_dims *input_dims,
    const int8_t *input_data, const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_avgpool_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_pool_params *pool_params, const cmsis_nn_dims *input_dims,
    const int8_t *input_data, const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc =
      __real_arm_avgpool_s8(ctx, pool_params, input_dims, input_data, filter_dims, output_dims, output_data);
  log_kernel("arm_avgpool_s8", toc_us(t0));
  return rc;
}

// arm_batch_matmul_s16
arm_cmsis_nn_status __real_arm_batch_matmul_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_bmm_params *bmm_params,
    const cmsis_nn_per_tensor_quant_params *quant_params, const cmsis_nn_dims *input_lhs_dims, const int16_t *input_lhs,
    const cmsis_nn_dims *input_rhs_dims, const int16_t *input_rhs, const cmsis_nn_dims *output_dims, int16_t *output);

arm_cmsis_nn_status __wrap_arm_batch_matmul_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_bmm_params *bmm_params,
    const cmsis_nn_per_tensor_quant_params *quant_params, const cmsis_nn_dims *input_lhs_dims, const int16_t *input_lhs,
    const cmsis_nn_dims *input_rhs_dims, const int16_t *input_rhs, const cmsis_nn_dims *output_dims, int16_t *output)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_batch_matmul_s16(
      ctx, bmm_params, quant_params, input_lhs_dims, input_lhs, input_rhs_dims, input_rhs, output_dims, output);
  capture_end_counters_and_log("arm_batch_matmul_s16", toc_us(t0), rc);
  log_kernel("arm_batch_matmul_s16", toc_us(t0));
  return rc;
}

// arm_batch_matmul_s8
arm_cmsis_nn_status __real_arm_batch_matmul_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_bmm_params *bmm_params,
    const cmsis_nn_per_tensor_quant_params *quant_params, const cmsis_nn_dims *input_lhs_dims, const int8_t *input_lhs,
    const cmsis_nn_dims *input_rhs_dims, const int8_t *input_rhs, const cmsis_nn_dims *output_dims, int8_t *output);

arm_cmsis_nn_status __wrap_arm_batch_matmul_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_bmm_params *bmm_params,
    const cmsis_nn_per_tensor_quant_params *quant_params, const cmsis_nn_dims *input_lhs_dims, const int8_t *input_lhs,
    const cmsis_nn_dims *input_rhs_dims, const int8_t *input_rhs, const cmsis_nn_dims *output_dims, int8_t *output)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_batch_matmul_s8(
      ctx, bmm_params, quant_params, input_lhs_dims, input_lhs, input_rhs_dims, input_rhs, output_dims, output);
  capture_end_counters_and_log("arm_batch_matmul_s8", toc_us(t0), rc);
  log_kernel("arm_batch_matmul_s8", toc_us(t0));
  return rc;
}

// arm_convolve_1_x_n_s4
arm_cmsis_nn_status __real_arm_convolve_1_x_n_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_convolve_1_x_n_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_convolve_1_x_n_s4(
      ctx, weight_sum_ctx, conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_convolve_1_x_n_s4", toc_us(t0), rc);
  log_kernel("arm_convolve_1_x_n_s4", toc_us(t0));
  return rc;
}

// arm_convolve_1_x_n_s8
arm_cmsis_nn_status __real_arm_convolve_1_x_n_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_convolve_1_x_n_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_convolve_1_x_n_s8(
      ctx, weight_sum_ctx, conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_convolve_1_x_n_s8", toc_us(t0), rc);
  log_kernel("arm_convolve_1_x_n_s8", toc_us(t0));
  return rc;
}

// arm_convolve_1x1_s4
arm_cmsis_nn_status __real_arm_convolve_1x1_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_convolve_1x1_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_convolve_1x1_s4(
      ctx, weight_sum_ctx, conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_convolve_1x1_s4", toc_us(t0), rc);
  log_kernel("arm_convolve_1x1_s4", toc_us(t0));
  return rc;
}

// arm_convolve_1x1_s4_fast
arm_cmsis_nn_status __real_arm_convolve_1x1_s4_fast(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_convolve_1x1_s4_fast(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_convolve_1x1_s4_fast(
      ctx, weight_sum_ctx, conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_convolve_1x1_s4_fast", toc_us(t0), rc);
  log_kernel("arm_convolve_1x1_s4_fast", toc_us(t0));
  return rc;
}

// arm_convolve_1x1_s8
arm_cmsis_nn_status __real_arm_convolve_1x1_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_convolve_1x1_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_convolve_1x1_s8(
      ctx, weight_sum_ctx, conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_convolve_1x1_s8", toc_us(t0), rc);
  log_kernel("arm_convolve_1x1_s8", toc_us(t0));
  return rc;
}

// arm_convolve_1x1_s8_fast
arm_cmsis_nn_status __real_arm_convolve_1x1_s8_fast(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_convolve_1x1_s8_fast(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_convolve_1x1_s8_fast(
      ctx, weight_sum_ctx, conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_convolve_1x1_s8_fast", toc_us(t0), rc);
  log_kernel("arm_convolve_1x1_s8_fast", toc_us(t0));
  return rc;
}

// arm_convolve_s16
arm_cmsis_nn_status __real_arm_convolve_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int16_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const cmsis_nn_bias_data *bias_data, const cmsis_nn_dims *output_dims, int16_t *output_data);

arm_cmsis_nn_status __wrap_arm_convolve_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int16_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const cmsis_nn_bias_data *bias_data, const cmsis_nn_dims *output_dims, int16_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_convolve_s16(
      ctx, weight_sum_ctx, conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_convolve_s16", toc_us(t0), rc);
  log_kernel("arm_convolve_s16", toc_us(t0));
  return rc;
}

// arm_convolve_s4
arm_cmsis_nn_status __real_arm_convolve_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_convolve_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_convolve_s4(
      ctx, weight_sum_ctx, conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_convolve_s4", toc_us(t0), rc);
  log_kernel("arm_convolve_s4", toc_us(t0));
  return rc;
}

// arm_convolve_s8
arm_cmsis_nn_status __real_arm_convolve_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *upscale_dims, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_convolve_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *upscale_dims, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_convolve_s8(
      ctx, weight_sum_ctx, conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, upscale_dims, output_dims, output_data);
  capture_end_counters_and_log("arm_convolve_s8", toc_us(t0), rc);
  log_kernel("arm_convolve_s8", toc_us(t0));
  return rc;
}

// arm_convolve_weight_sum
arm_cmsis_nn_status __real_arm_convolve_weight_sum(
    int32_t *vector_sum_buf, const int8_t *rhs, const cmsis_nn_dims *input_dims, const cmsis_nn_dims *filter_dims,
    const cmsis_nn_dims *output_dims, const int32_t lhs_offset, const int32_t *bias_data);

arm_cmsis_nn_status __wrap_arm_convolve_weight_sum(
    int32_t *vector_sum_buf, const int8_t *rhs, const cmsis_nn_dims *input_dims, const cmsis_nn_dims *filter_dims,
    const cmsis_nn_dims *output_dims, const int32_t lhs_offset, const int32_t *bias_data)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc =
      __real_arm_convolve_weight_sum(vector_sum_buf, rhs, input_dims, filter_dims, output_dims, lhs_offset, bias_data);
  log_kernel("arm_convolve_weight_sum", toc_us(t0));
  return rc;
}

// arm_convolve_weight_sum_s4
arm_cmsis_nn_status __real_arm_convolve_weight_sum_s4(
    int32_t *vector_sum_buf, const int8_t *weights_s4, const cmsis_nn_dims *input_dims,
    const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims, const int32_t lhs_offset,
    const int32_t *bias_data);

arm_cmsis_nn_status __wrap_arm_convolve_weight_sum_s4(
    int32_t *vector_sum_buf, const int8_t *weights_s4, const cmsis_nn_dims *input_dims,
    const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims, const int32_t lhs_offset,
    const int32_t *bias_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_convolve_weight_sum_s4(
      vector_sum_buf, weights_s4, input_dims, filter_dims, output_dims, lhs_offset, bias_data);
  capture_end_counters_and_log("arm_convolve_weight_sum_s4", toc_us(t0), rc);
  log_kernel("arm_convolve_weight_sum_s4", toc_us(t0));
  return rc;
}

// arm_convolve_wrapper_s16
arm_cmsis_nn_status __real_arm_convolve_wrapper_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int16_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const cmsis_nn_bias_data *bias_data, const cmsis_nn_dims *output_dims, int16_t *output_data);

arm_cmsis_nn_status __wrap_arm_convolve_wrapper_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int16_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const cmsis_nn_bias_data *bias_data, const cmsis_nn_dims *output_dims, int16_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_convolve_wrapper_s16(
      ctx, weight_sum_ctx, conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_convolve_wrapper_s16", toc_us(t0), rc);
  log_kernel("arm_convolve_wrapper_s16", toc_us(t0));
  return rc;
}

// arm_convolve_wrapper_s4
arm_cmsis_nn_status __real_arm_convolve_wrapper_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_convolve_wrapper_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_convolve_wrapper_s4(
      ctx, weight_sum_ctx, conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_convolve_wrapper_s4", toc_us(t0), rc);
  log_kernel("arm_convolve_wrapper_s4", toc_us(t0));
  return rc;
}

// arm_convolve_wrapper_s8
arm_cmsis_nn_status __real_arm_convolve_wrapper_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_convolve_wrapper_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_conv_params *conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_convolve_wrapper_s8(
      ctx, weight_sum_ctx, conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_convolve_wrapper_s8", toc_us(t0), rc);
  log_kernel("arm_convolve_wrapper_s8", toc_us(t0));
  return rc;
}

// arm_depthwise_conv_3x3_s8
arm_cmsis_nn_status __real_arm_depthwise_conv_3x3_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_depthwise_conv_3x3_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_depthwise_conv_3x3_s8(
      ctx, weight_sum_ctx, dw_conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_depthwise_conv_3x3_s8", toc_us(t0), rc);
  log_kernel("arm_depthwise_conv_3x3_s8", toc_us(t0));
  return rc;
}

// arm_depthwise_conv_fast_s16
arm_cmsis_nn_status __real_arm_depthwise_conv_fast_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int16_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int64_t *bias_data, const cmsis_nn_dims *output_dims, int16_t *output_data);

arm_cmsis_nn_status __wrap_arm_depthwise_conv_fast_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int16_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int64_t *bias_data, const cmsis_nn_dims *output_dims, int16_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_depthwise_conv_fast_s16(
      ctx, weight_sum_ctx, dw_conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_depthwise_conv_fast_s16", toc_us(t0), rc);
  log_kernel("arm_depthwise_conv_fast_s16", toc_us(t0));
  return rc;
}

// arm_depthwise_conv_s16
arm_cmsis_nn_status __real_arm_depthwise_conv_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int16_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int64_t *bias_data, const cmsis_nn_dims *output_dims, int16_t *output_data);

arm_cmsis_nn_status __wrap_arm_depthwise_conv_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int16_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int64_t *bias_data, const cmsis_nn_dims *output_dims, int16_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_depthwise_conv_s16(
      ctx, weight_sum_ctx, dw_conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_depthwise_conv_s16", toc_us(t0), rc);
  log_kernel("arm_depthwise_conv_s16", toc_us(t0));
  return rc;
}

// arm_depthwise_conv_s4
arm_cmsis_nn_status __real_arm_depthwise_conv_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input,
    const cmsis_nn_dims *filter_dims, const int8_t *kernel, const cmsis_nn_dims *bias_dims, const int32_t *bias,
    const cmsis_nn_dims *output_dims, int8_t *output);

arm_cmsis_nn_status __wrap_arm_depthwise_conv_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input,
    const cmsis_nn_dims *filter_dims, const int8_t *kernel, const cmsis_nn_dims *bias_dims, const int32_t *bias,
    const cmsis_nn_dims *output_dims, int8_t *output)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_depthwise_conv_s4(
      ctx, weight_sum_ctx, dw_conv_params, quant_params, input_dims, input, filter_dims, kernel, bias_dims, bias,
      output_dims, output);
  capture_end_counters_and_log("arm_depthwise_conv_s4", toc_us(t0), rc);
  log_kernel("arm_depthwise_conv_s4", toc_us(t0));
  return rc;
}

// arm_depthwise_conv_s8
arm_cmsis_nn_status __real_arm_depthwise_conv_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_depthwise_conv_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_depthwise_conv_s8(
      ctx, weight_sum_ctx, dw_conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_depthwise_conv_s8", toc_us(t0), rc);
  log_kernel("arm_depthwise_conv_s8", toc_us(t0));
  return rc;
}

// arm_depthwise_conv_s4_opt
arm_cmsis_nn_status __real_arm_depthwise_conv_s4_opt(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_depthwise_conv_s4_opt(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_depthwise_conv_s4_opt(
      ctx, weight_sum_ctx, dw_conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_depthwise_conv_s4_opt", toc_us(t0), rc);
  log_kernel("arm_depthwise_conv_s4_opt", toc_us(t0));
  return rc;
}

// arm_depthwise_conv_s8_opt
arm_cmsis_nn_status __real_arm_depthwise_conv_s8_opt(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_depthwise_conv_s8_opt(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_depthwise_conv_s8_opt(
      ctx, weight_sum_ctx, dw_conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_depthwise_conv_s8_opt", toc_us(t0), rc);
  log_kernel("arm_depthwise_conv_s8_opt", toc_us(t0));
  return rc;
}

// arm_depthwise_conv_wrapper_s16
arm_cmsis_nn_status __real_arm_depthwise_conv_wrapper_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int16_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int64_t *bias_data, const cmsis_nn_dims *output_dims, int16_t *output_data);

arm_cmsis_nn_status __wrap_arm_depthwise_conv_wrapper_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int16_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int64_t *bias_data, const cmsis_nn_dims *output_dims, int16_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_depthwise_conv_wrapper_s16(
      ctx, weight_sum_ctx, dw_conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_depthwise_conv_wrapper_s16", toc_us(t0), rc);
  log_kernel("arm_depthwise_conv_wrapper_s16", toc_us(t0));
  return rc;
}

// arm_depthwise_conv_wrapper_s4
arm_cmsis_nn_status __real_arm_depthwise_conv_wrapper_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_depthwise_conv_wrapper_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_depthwise_conv_wrapper_s4(
      ctx, weight_sum_ctx, dw_conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_depthwise_conv_wrapper_s4", toc_us(t0), rc);
  log_kernel("arm_depthwise_conv_wrapper_s4", toc_us(t0));
  return rc;
}

// arm_depthwise_conv_wrapper_s8
arm_cmsis_nn_status __real_arm_depthwise_conv_wrapper_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_depthwise_conv_wrapper_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_depthwise_conv_wrapper_s8(
      ctx, weight_sum_ctx, dw_conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_depthwise_conv_wrapper_s8", toc_us(t0), rc);
  log_kernel("arm_depthwise_conv_wrapper_s8", toc_us(t0));
  return rc;
}

// arm_depthwise_convolve_weight_sum
arm_cmsis_nn_status __real_arm_depthwise_convolve_weight_sum(
    int32_t *vector_sum_buf, int8_t *scratch_buf, const int8_t *rhs, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_dims *input_dims, const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims,
    const int32_t lhs_offset, const int32_t *bias_data);

arm_cmsis_nn_status __wrap_arm_depthwise_convolve_weight_sum(
    int32_t *vector_sum_buf, int8_t *scratch_buf, const int8_t *rhs, const cmsis_nn_dw_conv_params *dw_conv_params,
    const cmsis_nn_dims *input_dims, const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims,
    const int32_t lhs_offset, const int32_t *bias_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_depthwise_convolve_weight_sum(
      vector_sum_buf, scratch_buf, rhs, dw_conv_params, input_dims, filter_dims, output_dims, lhs_offset, bias_data);
  capture_end_counters_and_log("arm_depthwise_convolve_weight_sum", toc_us(t0), rc);
  log_kernel("arm_depthwise_convolve_weight_sum", toc_us(t0));
  return rc;
}

// arm_depthwise_weight_sum_s4
arm_cmsis_nn_status __real_arm_depthwise_weight_sum_s4(
    int32_t *vector_sum_buf, const int8_t *weights_s4, const cmsis_nn_dims *input_dims,
    const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims, const int32_t lhs_offset,
    const int32_t *bias_data);

arm_cmsis_nn_status __wrap_arm_depthwise_weight_sum_s4(
    int32_t *vector_sum_buf, const int8_t *weights_s4, const cmsis_nn_dims *input_dims,
    const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims, const int32_t lhs_offset,
    const int32_t *bias_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_depthwise_weight_sum_s4(
      vector_sum_buf, weights_s4, input_dims, filter_dims, output_dims, lhs_offset, bias_data);
  capture_end_counters_and_log("arm_depthwise_weight_sum_s4", toc_us(t0), rc);
  log_kernel("arm_depthwise_weight_sum_s4", toc_us(t0));
  return rc;
}

// arm_elementwise_add_s16
arm_cmsis_nn_status __real_arm_elementwise_add_s16(
    const int16_t *input_1_vect, const int16_t *input_2_vect, const int32_t input_1_offset, const int32_t input_1_mult,
    const int32_t input_1_shift, const int32_t input_2_offset, const int32_t input_2_mult, const int32_t input_2_shift,
    const int32_t left_shift, int16_t *output, const int32_t out_offset, const int32_t out_mult,
    const int32_t out_shift, const int32_t out_activation_min, const int32_t out_activation_max,
    const int32_t block_size);

arm_cmsis_nn_status __wrap_arm_elementwise_add_s16(
    const int16_t *input_1_vect, const int16_t *input_2_vect, const int32_t input_1_offset, const int32_t input_1_mult,
    const int32_t input_1_shift, const int32_t input_2_offset, const int32_t input_2_mult, const int32_t input_2_shift,
    const int32_t left_shift, int16_t *output, const int32_t out_offset, const int32_t out_mult,
    const int32_t out_shift, const int32_t out_activation_min, const int32_t out_activation_max,
    const int32_t block_size)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_elementwise_add_s16(
      input_1_vect, input_2_vect, input_1_offset, input_1_mult, input_1_shift, input_2_offset, input_2_mult,
      input_2_shift, left_shift, output, out_offset, out_mult, out_shift, out_activation_min, out_activation_max,
      block_size);
  capture_end_counters_and_log("arm_elementwise_add_s16", toc_us(t0), rc);
  log_kernel("arm_elementwise_add_s16", toc_us(t0));
  return rc;
}

// arm_elementwise_add_s8
arm_cmsis_nn_status __real_arm_elementwise_add_s8(
    const int8_t *input_1_vect, const int8_t *input_2_vect, const int32_t input_1_offset, const int32_t input_1_mult,
    const int32_t input_1_shift, const int32_t input_2_offset, const int32_t input_2_mult, const int32_t input_2_shift,
    const int32_t left_shift, int8_t *output, const int32_t out_offset, const int32_t out_mult, const int32_t out_shift,
    const int32_t out_activation_min, const int32_t out_activation_max, const int32_t block_size);

arm_cmsis_nn_status __wrap_arm_elementwise_add_s8(
    const int8_t *input_1_vect, const int8_t *input_2_vect, const int32_t input_1_offset, const int32_t input_1_mult,
    const int32_t input_1_shift, const int32_t input_2_offset, const int32_t input_2_mult, const int32_t input_2_shift,
    const int32_t left_shift, int8_t *output, const int32_t out_offset, const int32_t out_mult, const int32_t out_shift,
    const int32_t out_activation_min, const int32_t out_activation_max, const int32_t block_size)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_elementwise_add_s8(
      input_1_vect, input_2_vect, input_1_offset, input_1_mult, input_1_shift, input_2_offset, input_2_mult,
      input_2_shift, left_shift, output, out_offset, out_mult, out_shift, out_activation_min, out_activation_max,
      block_size);
  capture_end_counters_and_log("arm_elementwise_add_s8", toc_us(t0), rc);
  log_kernel("arm_elementwise_add_s8", toc_us(t0));
  return rc;
}

// arm_elementwise_mul_s16
arm_cmsis_nn_status __real_arm_elementwise_mul_s16(
    const int16_t *input_1_vect, const int16_t *input_2_vect, const int32_t input_1_offset,
    const int32_t input_2_offset, int16_t *output, const int32_t out_offset, const int32_t out_mult,
    const int32_t out_shift, const int32_t out_activation_min, const int32_t out_activation_max,
    const int32_t block_size);

arm_cmsis_nn_status __wrap_arm_elementwise_mul_s16(
    const int16_t *input_1_vect, const int16_t *input_2_vect, const int32_t input_1_offset,
    const int32_t input_2_offset, int16_t *output, const int32_t out_offset, const int32_t out_mult,
    const int32_t out_shift, const int32_t out_activation_min, const int32_t out_activation_max,
    const int32_t block_size)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_elementwise_mul_s16(
      input_1_vect, input_2_vect, input_1_offset, input_2_offset, output, out_offset, out_mult, out_shift,
      out_activation_min, out_activation_max, block_size);
  capture_end_counters_and_log("arm_elementwise_mul_s16", toc_us(t0), rc);
  log_kernel("arm_elementwise_mul_s16", toc_us(t0));
  return rc;
}

// arm_elementwise_mul_s8
arm_cmsis_nn_status __real_arm_elementwise_mul_s8(
    const int8_t *input_1_vect, const int8_t *input_2_vect, const int32_t input_1_offset, const int32_t input_2_offset,
    int8_t *output, const int32_t out_offset, const int32_t out_mult, const int32_t out_shift,
    const int32_t out_activation_min, const int32_t out_activation_max, const int32_t block_size);

arm_cmsis_nn_status __wrap_arm_elementwise_mul_s8(
    const int8_t *input_1_vect, const int8_t *input_2_vect, const int32_t input_1_offset, const int32_t input_2_offset,
    int8_t *output, const int32_t out_offset, const int32_t out_mult, const int32_t out_shift,
    const int32_t out_activation_min, const int32_t out_activation_max, const int32_t block_size)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_elementwise_mul_s8(
      input_1_vect, input_2_vect, input_1_offset, input_2_offset, output, out_offset, out_mult, out_shift,
      out_activation_min, out_activation_max, block_size);
  capture_end_counters_and_log("arm_elementwise_mul_s8", toc_us(t0), rc);
  log_kernel("arm_elementwise_mul_s8", toc_us(t0));
  return rc;
}

// arm_fully_connected_per_channel_s8
arm_cmsis_nn_status __real_arm_fully_connected_per_channel_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_fc_params *fc_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_fully_connected_per_channel_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_fc_params *fc_params,
    const cmsis_nn_per_channel_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_fully_connected_per_channel_s8(
      ctx, fc_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims, bias_data, output_dims,
      output_data);
  capture_end_counters_and_log("arm_fully_connected_per_channel_s8", toc_us(t0), rc);
  log_kernel("arm_fully_connected_per_channel_s8", toc_us(t0));
  return rc;
}

// arm_fully_connected_s16
arm_cmsis_nn_status __real_arm_fully_connected_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_fc_params *fc_params,
    const cmsis_nn_per_tensor_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int16_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int64_t *bias_data, const cmsis_nn_dims *output_dims, int16_t *output_data);

arm_cmsis_nn_status __wrap_arm_fully_connected_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_fc_params *fc_params,
    const cmsis_nn_per_tensor_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int16_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int64_t *bias_data, const cmsis_nn_dims *output_dims, int16_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_fully_connected_s16(
      ctx, fc_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims, bias_data, output_dims,
      output_data);
  capture_end_counters_and_log("arm_fully_connected_s16", toc_us(t0), rc);
  log_kernel("arm_fully_connected_s16", toc_us(t0));
  return rc;
}

// arm_fully_connected_s4
arm_cmsis_nn_status __real_arm_fully_connected_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_fc_params *fc_params,
    const cmsis_nn_per_tensor_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_fully_connected_s4(
    const cmsis_nn_context *ctx, const cmsis_nn_fc_params *fc_params,
    const cmsis_nn_per_tensor_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_fully_connected_s4(
      ctx, fc_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims, bias_data, output_dims,
      output_data);
  capture_end_counters_and_log("arm_fully_connected_s4", toc_us(t0), rc);
  log_kernel("arm_fully_connected_s4", toc_us(t0));
  return rc;
}

// arm_fully_connected_s8
arm_cmsis_nn_status __real_arm_fully_connected_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_fc_params *fc_params,
    const cmsis_nn_per_tensor_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_fully_connected_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_fc_params *fc_params,
    const cmsis_nn_per_tensor_quant_params *quant_params, const cmsis_nn_dims *input_dims, const int8_t *input_data,
    const cmsis_nn_dims *filter_dims, const int8_t *filter_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_fully_connected_s8(
      ctx, fc_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims, bias_data, output_dims,
      output_data);
  capture_end_counters_and_log("arm_fully_connected_s8", toc_us(t0), rc);
  log_kernel("arm_fully_connected_s8", toc_us(t0));
  return rc;
}

// arm_fully_connected_wrapper_s8
arm_cmsis_nn_status __real_arm_fully_connected_wrapper_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_fc_params *fc_params, const cmsis_nn_quant_params *quant_params,
    const cmsis_nn_dims *input_dims, const int8_t *input_data, const cmsis_nn_dims *filter_dims,
    const int8_t *filter_data, const cmsis_nn_dims *bias_dims, const int32_t *bias_data,
    const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_fully_connected_wrapper_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_fc_params *fc_params, const cmsis_nn_quant_params *quant_params,
    const cmsis_nn_dims *input_dims, const int8_t *input_data, const cmsis_nn_dims *filter_dims,
    const int8_t *filter_data, const cmsis_nn_dims *bias_dims, const int32_t *bias_data,
    const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_fully_connected_wrapper_s8(
      ctx, fc_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims, bias_data, output_dims,
      output_data);
  capture_end_counters_and_log("arm_fully_connected_wrapper_s8", toc_us(t0), rc);
  log_kernel("arm_fully_connected_wrapper_s8", toc_us(t0));
  return rc;
}

// arm_hard_swish_compat_s16
arm_cmsis_nn_status __real_arm_hard_swish_compat_s16(
    const int16_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier_fp,
    const int32_t output_multiplier_exp, const int32_t relu_multiplier_fp, const int32_t relu_multiplier_exp,
    int16_t *output, const int32_t output_size);

arm_cmsis_nn_status __wrap_arm_hard_swish_compat_s16(
    const int16_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier_fp,
    const int32_t output_multiplier_exp, const int32_t relu_multiplier_fp, const int32_t relu_multiplier_exp,
    int16_t *output, const int32_t output_size)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_hard_swish_compat_s16(
      input, input_offset, output_offset, output_multiplier_fp, output_multiplier_exp, relu_multiplier_fp,
      relu_multiplier_exp, output, output_size);
  capture_end_counters_and_log("arm_hard_swish_compat_s16", toc_us(t0), rc);
  log_kernel("arm_hard_swish_compat_s16", toc_us(t0));
  return rc;
}

// arm_hard_swish_compat_s8
arm_cmsis_nn_status __real_arm_hard_swish_compat_s8(
    const int8_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier_fp,
    const int32_t output_multiplier_exp, const int32_t relu_multiplier_fp, const int32_t relu_multiplier_exp,
    int8_t *output, const int32_t output_size);

arm_cmsis_nn_status __wrap_arm_hard_swish_compat_s8(
    const int8_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier_fp,
    const int32_t output_multiplier_exp, const int32_t relu_multiplier_fp, const int32_t relu_multiplier_exp,
    int8_t *output, const int32_t output_size)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_hard_swish_compat_s8(
      input, input_offset, output_offset, output_multiplier_fp, output_multiplier_exp, relu_multiplier_fp,
      relu_multiplier_exp, output, output_size);
  capture_end_counters_and_log("arm_hard_swish_compat_s8", toc_us(t0), rc);
  log_kernel("arm_hard_swish_compat_s8", toc_us(t0));
  return rc;
}

// arm_hard_swish_precise_s16
arm_cmsis_nn_status __real_arm_hard_swish_precise_s16(
    const int16_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier,
    const int32_t output_shift, const int32_t relu_q3, const int32_t relu_q6, int16_t *output,
    const int32_t output_size);

arm_cmsis_nn_status __wrap_arm_hard_swish_precise_s16(
    const int16_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier,
    const int32_t output_shift, const int32_t relu_q3, const int32_t relu_q6, int16_t *output,
    const int32_t output_size)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_hard_swish_precise_s16(
      input, input_offset, output_offset, output_multiplier, output_shift, relu_q3, relu_q6, output, output_size);
  capture_end_counters_and_log("arm_hard_swish_precise_s16", toc_us(t0), rc);
  log_kernel("arm_hard_swish_precise_s16", toc_us(t0));
  return rc;
}

// arm_hard_swish_precise_s8
arm_cmsis_nn_status __real_arm_hard_swish_precise_s8(
    const int8_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier,
    const int32_t output_shift, const int32_t relu_q3, const int32_t relu_q6, int8_t *output,
    const int32_t output_size);

arm_cmsis_nn_status __wrap_arm_hard_swish_precise_s8(
    const int8_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier,
    const int32_t output_shift, const int32_t relu_q3, const int32_t relu_q6, int8_t *output, const int32_t output_size)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_hard_swish_precise_s8(
      input, input_offset, output_offset, output_multiplier, output_shift, relu_q3, relu_q6, output, output_size);
  capture_end_counters_and_log("arm_hard_swish_precise_s8", toc_us(t0), rc);
  log_kernel("arm_hard_swish_precise_s8", toc_us(t0));
  return rc;
}

// arm_leaky_relu_s8
arm_cmsis_nn_status __real_arm_leaky_relu_s8(
    const int8_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier_alpha,
    const int32_t output_shift_alpha, const int32_t output_multiplier_identity, const int32_t output_shift_identity,
    int8_t *output, const int32_t output_size);

arm_cmsis_nn_status __wrap_arm_leaky_relu_s8(
    const int8_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier_alpha,
    const int32_t output_shift_alpha, const int32_t output_multiplier_identity, const int32_t output_shift_identity,
    int8_t *output, const int32_t output_size)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_leaky_relu_s8(
      input, input_offset, output_offset, output_multiplier_alpha, output_shift_alpha, output_multiplier_identity,
      output_shift_identity, output, output_size);
  capture_end_counters_and_log("arm_leaky_relu_s8", toc_us(t0), rc);
  log_kernel("arm_leaky_relu_s8", toc_us(t0));
  return rc;
}

// arm_lstm_unidirectional_s16
arm_cmsis_nn_status __real_arm_lstm_unidirectional_s16(
    const int16_t *input, int16_t *output, const cmsis_nn_lstm_params *params, cmsis_nn_lstm_context *buffers);

arm_cmsis_nn_status __wrap_arm_lstm_unidirectional_s16(
    const int16_t *input, int16_t *output, const cmsis_nn_lstm_params *params, cmsis_nn_lstm_context *buffers)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_lstm_unidirectional_s16(input, output, params, buffers);
  capture_end_counters_and_log("arm_lstm_unidirectional_s16", toc_us(t0), rc);
  log_kernel("arm_lstm_unidirectional_s16", toc_us(t0));
  return rc;
}

// arm_lstm_unidirectional_s8
arm_cmsis_nn_status __real_arm_lstm_unidirectional_s8(
    const int8_t *input, int8_t *output, const cmsis_nn_lstm_params *params, cmsis_nn_lstm_context *buffers);

arm_cmsis_nn_status __wrap_arm_lstm_unidirectional_s8(
    const int8_t *input, int8_t *output, const cmsis_nn_lstm_params *params, cmsis_nn_lstm_context *buffers)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_lstm_unidirectional_s8(input, output, params, buffers);
  capture_end_counters_and_log("arm_lstm_unidirectional_s8", toc_us(t0), rc);
  log_kernel("arm_lstm_unidirectional_s8", toc_us(t0));
  return rc;
}

// arm_max_pool_s16
arm_cmsis_nn_status __real_arm_max_pool_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_pool_params *pool_params, const cmsis_nn_dims *input_dims,
    const int16_t *src, const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims, int16_t *dst);

arm_cmsis_nn_status __wrap_arm_max_pool_s16(
    const cmsis_nn_context *ctx, const cmsis_nn_pool_params *pool_params, const cmsis_nn_dims *input_dims,
    const int16_t *src, const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims, int16_t *dst)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_max_pool_s16(ctx, pool_params, input_dims, src, filter_dims, output_dims, dst);
  capture_end_counters_and_log("arm_max_pool_s16", toc_us(t0), rc);
  log_kernel("arm_max_pool_s16", toc_us(t0));
  return rc;
}

// arm_max_pool_s8
arm_cmsis_nn_status __real_arm_max_pool_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_pool_params *pool_params, const cmsis_nn_dims *input_dims,
    const int8_t *input_data, const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_max_pool_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_pool_params *pool_params, const cmsis_nn_dims *input_dims,
    const int8_t *input_data, const cmsis_nn_dims *filter_dims, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc =
      __real_arm_max_pool_s8(ctx, pool_params, input_dims, input_data, filter_dims, output_dims, output_data);
  log_kernel("arm_max_pool_s8", toc_us(t0));
  return rc;
}

// arm_maximum_s16
arm_cmsis_nn_status __real_arm_maximum_s16(
    const cmsis_nn_context *ctx, const int16_t *input_1_data, const cmsis_nn_dims *input_1_dims,
    const int16_t *input_2_data, const cmsis_nn_dims *input_2_dims, int16_t *output_data,
    const cmsis_nn_dims *output_dims);

arm_cmsis_nn_status __wrap_arm_maximum_s16(
    const cmsis_nn_context *ctx, const int16_t *input_1_data, const cmsis_nn_dims *input_1_dims,
    const int16_t *input_2_data, const cmsis_nn_dims *input_2_dims, int16_t *output_data,
    const cmsis_nn_dims *output_dims)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc =
      __real_arm_maximum_s16(ctx, input_1_data, input_1_dims, input_2_data, input_2_dims, output_data, output_dims);
  log_kernel("arm_maximum_s16", toc_us(t0));
  return rc;
}

// arm_maximum_s8
arm_cmsis_nn_status __real_arm_maximum_s8(
    const cmsis_nn_context *ctx, const int8_t *input_1_data, const cmsis_nn_dims *input_1_dims,
    const int8_t *input_2_data, const cmsis_nn_dims *input_2_dims, int8_t *output_data,
    const cmsis_nn_dims *output_dims);

arm_cmsis_nn_status __wrap_arm_maximum_s8(
    const cmsis_nn_context *ctx, const int8_t *input_1_data, const cmsis_nn_dims *input_1_dims,
    const int8_t *input_2_data, const cmsis_nn_dims *input_2_dims, int8_t *output_data,
    const cmsis_nn_dims *output_dims)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc =
      __real_arm_maximum_s8(ctx, input_1_data, input_1_dims, input_2_data, input_2_dims, output_data, output_dims);
  log_kernel("arm_maximum_s8", toc_us(t0));
  return rc;
}

// arm_mean_s16
arm_cmsis_nn_status __real_arm_mean_s16(
    const int16_t *input_data, const cmsis_nn_dims *input_dims, const int32_t input_offset,
    const cmsis_nn_dims *axis_dims, int16_t *output_data, const cmsis_nn_dims *output_dims, const int32_t out_offset,
    const int32_t out_mult, const int32_t out_shift);

arm_cmsis_nn_status __wrap_arm_mean_s16(
    const int16_t *input_data, const cmsis_nn_dims *input_dims, const int32_t input_offset,
    const cmsis_nn_dims *axis_dims, int16_t *output_data, const cmsis_nn_dims *output_dims, const int32_t out_offset,
    const int32_t out_mult, const int32_t out_shift)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_mean_s16(
      input_data, input_dims, input_offset, axis_dims, output_data, output_dims, out_offset, out_mult, out_shift);
  capture_end_counters_and_log("arm_mean_s16", toc_us(t0), rc);
  log_kernel("arm_mean_s16", toc_us(t0));
  return rc;
}

// arm_mean_s8
arm_cmsis_nn_status __real_arm_mean_s8(
    const int8_t *input_data, const cmsis_nn_dims *input_dims, const int32_t input_offset,
    const cmsis_nn_dims *axis_dims, int8_t *output_data, const cmsis_nn_dims *output_dims, const int32_t out_offset,
    const int32_t out_mult, const int32_t out_shift);

arm_cmsis_nn_status __wrap_arm_mean_s8(
    const int8_t *input_data, const cmsis_nn_dims *input_dims, const int32_t input_offset,
    const cmsis_nn_dims *axis_dims, int8_t *output_data, const cmsis_nn_dims *output_dims, const int32_t out_offset,
    const int32_t out_mult, const int32_t out_shift)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_mean_s8(
      input_data, input_dims, input_offset, axis_dims, output_data, output_dims, out_offset, out_mult, out_shift);
  capture_end_counters_and_log("arm_mean_s8", toc_us(t0), rc);
  log_kernel("arm_mean_s8", toc_us(t0));
  return rc;
}

// arm_minimum_s16
arm_cmsis_nn_status __real_arm_minimum_s16(
    const cmsis_nn_context *ctx, const int16_t *input_1_data, const cmsis_nn_dims *input_1_dims,
    const int16_t *input_2_data, const cmsis_nn_dims *input_2_dims, int16_t *output_data,
    const cmsis_nn_dims *output_dims);

arm_cmsis_nn_status __wrap_arm_minimum_s16(
    const cmsis_nn_context *ctx, const int16_t *input_1_data, const cmsis_nn_dims *input_1_dims,
    const int16_t *input_2_data, const cmsis_nn_dims *input_2_dims, int16_t *output_data,
    const cmsis_nn_dims *output_dims)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc =
      __real_arm_minimum_s16(ctx, input_1_data, input_1_dims, input_2_data, input_2_dims, output_data, output_dims);
  log_kernel("arm_minimum_s16", toc_us(t0));
  return rc;
}

// arm_minimum_s8
arm_cmsis_nn_status __real_arm_minimum_s8(
    const cmsis_nn_context *ctx, const int8_t *input_1_data, const cmsis_nn_dims *input_1_dims,
    const int8_t *input_2_data, const cmsis_nn_dims *input_2_dims, int8_t *output_data,
    const cmsis_nn_dims *output_dims);

arm_cmsis_nn_status __wrap_arm_minimum_s8(
    const cmsis_nn_context *ctx, const int8_t *input_1_data, const cmsis_nn_dims *input_1_dims,
    const int8_t *input_2_data, const cmsis_nn_dims *input_2_dims, int8_t *output_data,
    const cmsis_nn_dims *output_dims)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc =
      __real_arm_minimum_s8(ctx, input_1_data, input_1_dims, input_2_data, input_2_dims, output_data, output_dims);
  log_kernel("arm_minimum_s8", toc_us(t0));
  return rc;
}

// arm_mul_s16
arm_cmsis_nn_status __real_arm_mul_s16(
    const int16_t *input1_data, const cmsis_nn_dims *input1_dims, const int16_t *input2_data,
    const cmsis_nn_dims *input2_dims, const int32_t input1_offset, const int32_t input2_offset, int16_t *output_data,
    const cmsis_nn_dims *output_dims, const int32_t out_offset, const int32_t out_mult, const int32_t out_shift,
    const int32_t out_activation_min, const int32_t out_activation_max);

arm_cmsis_nn_status __wrap_arm_mul_s16(
    const int16_t *input1_data, const cmsis_nn_dims *input1_dims, const int16_t *input2_data,
    const cmsis_nn_dims *input2_dims, const int32_t input1_offset, const int32_t input2_offset, int16_t *output_data,
    const cmsis_nn_dims *output_dims, const int32_t out_offset, const int32_t out_mult, const int32_t out_shift,
    const int32_t out_activation_min, const int32_t out_activation_max)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_mul_s16(
      input1_data, input1_dims, input2_data, input2_dims, input1_offset, input2_offset, output_data, output_dims,
      out_offset, out_mult, out_shift, out_activation_min, out_activation_max);
  capture_end_counters_and_log("arm_mul_s16", toc_us(t0), rc);
  log_kernel("arm_mul_s16", toc_us(t0));
  return rc;
}

// arm_mul_s8
arm_cmsis_nn_status __real_arm_mul_s8(
    const int8_t *input1_data, const cmsis_nn_dims *input1_dims, const int8_t *input2_data,
    const cmsis_nn_dims *input2_dims, const int32_t input1_offset, const int32_t input2_offset, int8_t *output_data,
    const cmsis_nn_dims *output_dims, const int32_t out_offset, const int32_t out_mult, const int32_t out_shift,
    const int32_t out_activation_min, const int32_t out_activation_max);

arm_cmsis_nn_status __wrap_arm_mul_s8(
    const int8_t *input1_data, const cmsis_nn_dims *input1_dims, const int8_t *input2_data,
    const cmsis_nn_dims *input2_dims, const int32_t input1_offset, const int32_t input2_offset, int8_t *output_data,
    const cmsis_nn_dims *output_dims, const int32_t out_offset, const int32_t out_mult, const int32_t out_shift,
    const int32_t out_activation_min, const int32_t out_activation_max)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_mul_s8(
      input1_data, input1_dims, input2_data, input2_dims, input1_offset, input2_offset, output_data, output_dims,
      out_offset, out_mult, out_shift, out_activation_min, out_activation_max);
  capture_end_counters_and_log("arm_mul_s8", toc_us(t0), rc);
  log_kernel("arm_mul_s8", toc_us(t0));
  return rc;
}

// arm_pad_s16
arm_cmsis_nn_status __real_arm_pad_s16(
    const int16_t *input, int16_t *output, const int16_t pad_value, const cmsis_nn_dims *input_size,
    const cmsis_nn_dims *pre_pad, const cmsis_nn_dims *post_pad);

arm_cmsis_nn_status __wrap_arm_pad_s16(
    const int16_t *input, int16_t *output, const int16_t pad_value, const cmsis_nn_dims *input_size,
    const cmsis_nn_dims *pre_pad, const cmsis_nn_dims *post_pad)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_pad_s16(input, output, pad_value, input_size, pre_pad, post_pad);
  capture_end_counters_and_log("arm_pad_s16", toc_us(t0), rc);
  log_kernel("arm_pad_s16", toc_us(t0));
  return rc;
}

// arm_pad_s8
arm_cmsis_nn_status __real_arm_pad_s8(
    const int8_t *input, int8_t *output, const int8_t pad_value, const cmsis_nn_dims *input_size,
    const cmsis_nn_dims *pre_pad, const cmsis_nn_dims *post_pad);

arm_cmsis_nn_status __wrap_arm_pad_s8(
    const int8_t *input, int8_t *output, const int8_t pad_value, const cmsis_nn_dims *input_size,
    const cmsis_nn_dims *pre_pad, const cmsis_nn_dims *post_pad)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_pad_s8(input, output, pad_value, input_size, pre_pad, post_pad);
  capture_end_counters_and_log("arm_pad_s8", toc_us(t0), rc);
  log_kernel("arm_pad_s8", toc_us(t0));
  return rc;
}

// arm_quantize_f32_s16
arm_cmsis_nn_status
__real_arm_quantize_f32_s16(const float *input, int16_t *output, int32_t size, int32_t zero_point, float scale);

arm_cmsis_nn_status
__wrap_arm_quantize_f32_s16(const float *input, int16_t *output, int32_t size, int32_t zero_point, float scale)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc = __real_arm_quantize_f32_s16(input, output, size, zero_point, scale);
  log_kernel("arm_quantize_f32_s16", toc_us(t0));
  return rc;
}

// arm_quantize_f32_s8
arm_cmsis_nn_status
__real_arm_quantize_f32_s8(const float *input, int8_t *output, int32_t size, int32_t zero_point, float scale);

arm_cmsis_nn_status
__wrap_arm_quantize_f32_s8(const float *input, int8_t *output, int32_t size, int32_t zero_point, float scale)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc = __real_arm_quantize_f32_s8(input, output, size, zero_point, scale);
  log_kernel("arm_quantize_f32_s8", toc_us(t0));
  return rc;
}

// arm_reduce_max_s16
arm_cmsis_nn_status __real_arm_reduce_max_s16(
    const int16_t *input_data, const cmsis_nn_dims *input_dims, const cmsis_nn_dims *axis_dims, int16_t *output_data,
    const cmsis_nn_dims *output_dims);

arm_cmsis_nn_status __wrap_arm_reduce_max_s16(
    const int16_t *input_data, const cmsis_nn_dims *input_dims, const cmsis_nn_dims *axis_dims, int16_t *output_data,
    const cmsis_nn_dims *output_dims)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_reduce_max_s16(input_data, input_dims, axis_dims, output_data, output_dims);
  capture_end_counters_and_log("arm_reduce_max_s16", toc_us(t0), rc);
  log_kernel("arm_reduce_max_s16", toc_us(t0));
  return rc;
}

// arm_reduce_max_s8
arm_cmsis_nn_status __real_arm_reduce_max_s8(
    const int8_t *input_data, const cmsis_nn_dims *input_dims, const cmsis_nn_dims *axis_dims, int8_t *output_data,
    const cmsis_nn_dims *output_dims);

arm_cmsis_nn_status __wrap_arm_reduce_max_s8(
    const int8_t *input_data, const cmsis_nn_dims *input_dims, const cmsis_nn_dims *axis_dims, int8_t *output_data,
    const cmsis_nn_dims *output_dims)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_reduce_max_s8(input_data, input_dims, axis_dims, output_data, output_dims);
  capture_end_counters_and_log("arm_reduce_max_s8", toc_us(t0), rc);
  log_kernel("arm_reduce_max_s8", toc_us(t0));
  return rc;
}

// arm_relu_s16
arm_cmsis_nn_status __real_arm_relu_s16(
    const int16_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier,
    const int32_t output_shift, int16_t *output, const int32_t output_size);

arm_cmsis_nn_status __wrap_arm_relu_s16(
    const int16_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier,
    const int32_t output_shift, int16_t *output, const int32_t output_size)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc =
      __real_arm_relu_s16(input, input_offset, output_offset, output_multiplier, output_shift, output, output_size);
  log_kernel("arm_relu_s16", toc_us(t0));
  return rc;
}

// arm_relu_s8
arm_cmsis_nn_status __real_arm_relu_s8(
    const int8_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier,
    const int32_t output_shift, int8_t *output, const int32_t output_size);

arm_cmsis_nn_status __wrap_arm_relu_s8(
    const int8_t *input, const int32_t input_offset, const int32_t output_offset, const int32_t output_multiplier,
    const int32_t output_shift, int8_t *output, const int32_t output_size)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc =
      __real_arm_relu_s8(input, input_offset, output_offset, output_multiplier, output_shift, output, output_size);
  log_kernel("arm_relu_s8", toc_us(t0));
  return rc;
}

// arm_requantize_s16_s16
arm_cmsis_nn_status __real_arm_requantize_s16_s16(
    const int16_t *input, int16_t *output, int32_t size, int32_t effective_scale_multiplier,
    int32_t effective_scale_shift, int32_t input_zeropoint, int32_t output_zeropoint);

arm_cmsis_nn_status __wrap_arm_requantize_s16_s16(
    const int16_t *input, int16_t *output, int32_t size, int32_t effective_scale_multiplier,
    int32_t effective_scale_shift, int32_t input_zeropoint, int32_t output_zeropoint)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_requantize_s16_s16(
      input, output, size, effective_scale_multiplier, effective_scale_shift, input_zeropoint, output_zeropoint);
  capture_end_counters_and_log("arm_requantize_s16_s16", toc_us(t0), rc);
  log_kernel("arm_requantize_s16_s16", toc_us(t0));
  return rc;
}

// arm_requantize_s8_s8
arm_cmsis_nn_status __real_arm_requantize_s8_s8(
    const int8_t *input, int8_t *output, int32_t size, int32_t effective_scale_multiplier,
    int32_t effective_scale_shift, int32_t input_zeropoint, int32_t output_zeropoint);

arm_cmsis_nn_status __wrap_arm_requantize_s8_s8(
    const int8_t *input, int8_t *output, int32_t size, int32_t effective_scale_multiplier,
    int32_t effective_scale_shift, int32_t input_zeropoint, int32_t output_zeropoint)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_requantize_s8_s8(
      input, output, size, effective_scale_multiplier, effective_scale_shift, input_zeropoint, output_zeropoint);
  capture_end_counters_and_log("arm_requantize_s8_s8", toc_us(t0), rc);
  log_kernel("arm_requantize_s8_s8", toc_us(t0));
  return rc;
}

// arm_softmax_s16
arm_cmsis_nn_status __real_arm_softmax_s16(
    const int16_t *input, const int32_t num_rows, const int32_t row_size, const int32_t mult, const int32_t shift,
    const cmsis_nn_softmax_lut_s16 *softmax_params, int16_t *output);

arm_cmsis_nn_status __wrap_arm_softmax_s16(
    const int16_t *input, const int32_t num_rows, const int32_t row_size, const int32_t mult, const int32_t shift,
    const cmsis_nn_softmax_lut_s16 *softmax_params, int16_t *output)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_softmax_s16(input, num_rows, row_size, mult, shift, softmax_params, output);
  capture_end_counters_and_log("arm_softmax_s16", toc_us(t0), rc);
  log_kernel("arm_softmax_s16", toc_us(t0));
  return rc;
}

// arm_softmax_s8
arm_cmsis_nn_status __real_arm_softmax_s8(
    const int8_t *input, const int32_t num_rows, const int32_t row_size, const int32_t mult, const int32_t shift,
    const int32_t diff_min, int8_t *output);

    arm_cmsis_nn_status __wrap_arm_softmax_s8(
    const int8_t *input, const int32_t num_rows, const int32_t row_size, const int32_t mult, const int32_t shift,
    const int32_t diff_min, int8_t *output)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
    arm_cmsis_nn_status rc = __real_arm_softmax_s8(input, num_rows, row_size, mult, shift, diff_min, output);
  capture_end_counters_and_log("arm_softmax_s8", toc_us(t0), rc);
  log_kernel("arm_softmax_s8", toc_us(t0));
  return rc;
  }

// arm_softmax_s8_s16
arm_cmsis_nn_status __real_arm_softmax_s8_s16(
    const int8_t *input, const int32_t num_rows, const int32_t row_size, const int32_t mult, const int32_t shift,
    const int32_t diff_min, int16_t *output);

    arm_cmsis_nn_status __wrap_arm_softmax_s8_s16(
    const int8_t *input, const int32_t num_rows, const int32_t row_size, const int32_t mult, const int32_t shift,
    const int32_t diff_min, int16_t *output)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
    arm_cmsis_nn_status rc =  __real_arm_softmax_s8_s16(input, num_rows, row_size, mult, shift, diff_min, output);
  capture_end_counters_and_log("arm_softmax_s8_s16", toc_us(t0), rc);
  log_kernel("arm_softmax_s8_s16", toc_us(t0));
  return rc;
  }

// arm_strided_slice_s8
arm_cmsis_nn_status __real_arm_strided_slice_s8(
    const int8_t *input_data, int8_t *output_data, const cmsis_nn_dims *const input_dims,
    const cmsis_nn_dims *const begin_dims, const cmsis_nn_dims *const stride_dims,
    const cmsis_nn_dims *const output_dims);

arm_cmsis_nn_status __wrap_arm_strided_slice_s8(
    const int8_t *input_data, int8_t *output_data, const cmsis_nn_dims *const input_dims,
    const cmsis_nn_dims *const begin_dims, const cmsis_nn_dims *const stride_dims,
    const cmsis_nn_dims *const output_dims)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc =
      __real_arm_strided_slice_s8(input_data, output_data, input_dims, begin_dims, stride_dims, output_dims);
  log_kernel("arm_strided_slice_s8", toc_us(t0));
  return rc;
}

// arm_svdf_s8
arm_cmsis_nn_status __real_arm_svdf_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *input_ctx, const cmsis_nn_context *output_ctx,
    const cmsis_nn_svdf_params *svdf_params, const cmsis_nn_per_tensor_quant_params *input_quant_params,
    const cmsis_nn_per_tensor_quant_params *output_quant_params, const cmsis_nn_dims *input_dims,
    const int8_t *input_data, const cmsis_nn_dims *state_dims, int8_t *state_data,
    const cmsis_nn_dims *weights_feature_dims, const int8_t *weights_feature_data,
    const cmsis_nn_dims *weights_time_dims, const int8_t *weights_time_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_svdf_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *input_ctx, const cmsis_nn_context *output_ctx,
    const cmsis_nn_svdf_params *svdf_params, const cmsis_nn_per_tensor_quant_params *input_quant_params,
    const cmsis_nn_per_tensor_quant_params *output_quant_params, const cmsis_nn_dims *input_dims,
    const int8_t *input_data, const cmsis_nn_dims *state_dims, int8_t *state_data,
    const cmsis_nn_dims *weights_feature_dims, const int8_t *weights_feature_data,
    const cmsis_nn_dims *weights_time_dims, const int8_t *weights_time_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_svdf_s8(
      ctx, input_ctx, output_ctx, svdf_params, input_quant_params, output_quant_params, input_dims, input_data,
      state_dims, state_data, weights_feature_dims, weights_feature_data, weights_time_dims, weights_time_data,
      bias_dims, bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_svdf_s8", toc_us(t0), rc);
  log_kernel("arm_svdf_s8", toc_us(t0));
  return rc;
}

// arm_svdf_state_s16_s8
arm_cmsis_nn_status __real_arm_svdf_state_s16_s8(
    const cmsis_nn_context *input_ctx, const cmsis_nn_context *output_ctx, const cmsis_nn_svdf_params *svdf_params,
    const cmsis_nn_per_tensor_quant_params *input_quant_params,
    const cmsis_nn_per_tensor_quant_params *output_quant_params, const cmsis_nn_dims *input_dims,
    const int8_t *input_data, const cmsis_nn_dims *state_dims, int16_t *state_data,
    const cmsis_nn_dims *weights_feature_dims, const int8_t *weights_feature_data,
    const cmsis_nn_dims *weights_time_dims, const int16_t *weights_time_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_svdf_state_s16_s8(
    const cmsis_nn_context *input_ctx, const cmsis_nn_context *output_ctx, const cmsis_nn_svdf_params *svdf_params,
    const cmsis_nn_per_tensor_quant_params *input_quant_params,
    const cmsis_nn_per_tensor_quant_params *output_quant_params, const cmsis_nn_dims *input_dims,
    const int8_t *input_data, const cmsis_nn_dims *state_dims, int16_t *state_data,
    const cmsis_nn_dims *weights_feature_dims, const int8_t *weights_feature_data,
    const cmsis_nn_dims *weights_time_dims, const int16_t *weights_time_data, const cmsis_nn_dims *bias_dims,
    const int32_t *bias_data, const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_svdf_state_s16_s8(
      input_ctx, output_ctx, svdf_params, input_quant_params, output_quant_params, input_dims, input_data, state_dims,
      state_data, weights_feature_dims, weights_feature_data, weights_time_dims, weights_time_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_svdf_state_s16_s8", toc_us(t0), rc);
  log_kernel("arm_svdf_state_s16_s8", toc_us(t0));
  return rc;
}

// arm_transpose_conv_s8
arm_cmsis_nn_status __real_arm_transpose_conv_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *output_ctx,
    const cmsis_nn_transpose_conv_params *transpose_conv_params, const cmsis_nn_per_channel_quant_params *quant_params,
    const cmsis_nn_dims *input_dims, const int8_t *input_data, const cmsis_nn_dims *filter_dims,
    const int8_t *filter_data, const cmsis_nn_dims *bias_dims, const int32_t *bias_data,
    const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_transpose_conv_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *output_ctx,
    const cmsis_nn_transpose_conv_params *transpose_conv_params, const cmsis_nn_per_channel_quant_params *quant_params,
    const cmsis_nn_dims *input_dims, const int8_t *input_data, const cmsis_nn_dims *filter_dims,
    const int8_t *filter_data, const cmsis_nn_dims *bias_dims, const int32_t *bias_data,
    const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_transpose_conv_s8(
      ctx, output_ctx, transpose_conv_params, quant_params, input_dims, input_data, filter_dims, filter_data, bias_dims,
      bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_transpose_conv_s8", toc_us(t0), rc);
  log_kernel("arm_transpose_conv_s8", toc_us(t0));
  return rc;
}

// arm_transpose_conv_wrapper_s8
arm_cmsis_nn_status __real_arm_transpose_conv_wrapper_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_context *output_ctx,
    const cmsis_nn_transpose_conv_params *transpose_conv_params, const cmsis_nn_per_channel_quant_params *quant_params,
    const cmsis_nn_dims *input_dims, const int8_t *input_data, const cmsis_nn_dims *filter_dims,
    const int8_t *filter_data, const cmsis_nn_dims *bias_dims, const int32_t *bias_data,
    const cmsis_nn_dims *output_dims, int8_t *output_data);

arm_cmsis_nn_status __wrap_arm_transpose_conv_wrapper_s8(
    const cmsis_nn_context *ctx, const cmsis_nn_context *weight_sum_ctx, const cmsis_nn_context *output_ctx,
    const cmsis_nn_transpose_conv_params *transpose_conv_params, const cmsis_nn_per_channel_quant_params *quant_params,
    const cmsis_nn_dims *input_dims, const int8_t *input_data, const cmsis_nn_dims *filter_dims,
    const int8_t *filter_data, const cmsis_nn_dims *bias_dims, const int32_t *bias_data,
    const cmsis_nn_dims *output_dims, int8_t *output_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_transpose_conv_wrapper_s8(
      ctx, weight_sum_ctx, output_ctx, transpose_conv_params, quant_params, input_dims, input_data, filter_dims,
      filter_data, bias_dims, bias_data, output_dims, output_data);
  capture_end_counters_and_log("arm_transpose_conv_wrapper_s8", toc_us(t0), rc);
  log_kernel("arm_transpose_conv_wrapper_s8", toc_us(t0));
  return rc;
}

// arm_transpose_s16
arm_cmsis_nn_status __real_arm_transpose_s16(
    const int16_t *input_data, int16_t *const output_data, const cmsis_nn_dims *const input_dims,
    const cmsis_nn_dims *const output_dims, const cmsis_nn_transpose_params *const transpose_params);

arm_cmsis_nn_status __wrap_arm_transpose_s16(
    const int16_t *input_data, int16_t *const output_data, const cmsis_nn_dims *const input_dims,
    const cmsis_nn_dims *const output_dims, const cmsis_nn_transpose_params *const transpose_params)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_transpose_s16(input_data, output_data, input_dims, output_dims, transpose_params);
  capture_end_counters_and_log("arm_transpose_s16", toc_us(t0), rc);
  log_kernel("arm_transpose_s16", toc_us(t0));
  return rc;
}

// arm_transpose_s8
arm_cmsis_nn_status __real_arm_transpose_s8(
    const int8_t *input_data, int8_t *const output_data, const cmsis_nn_dims *const input_dims,
    const cmsis_nn_dims *const output_dims, const cmsis_nn_transpose_params *const transpose_params);

arm_cmsis_nn_status __wrap_arm_transpose_s8(
    const int8_t *input_data, int8_t *const output_data, const cmsis_nn_dims *const input_dims,
    const cmsis_nn_dims *const output_dims, const cmsis_nn_transpose_params *const transpose_params)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_transpose_s8(input_data, output_data, input_dims, output_dims, transpose_params);
  capture_end_counters_and_log("arm_transpose_s8", toc_us(t0), rc);
  log_kernel("arm_transpose_s8", toc_us(t0));
  return rc;
}

// arm_vector_sum_s4
arm_cmsis_nn_status __real_arm_vector_sum_s4(
    int32_t *vector_sum_buf, const int8_t *weights_s4, const cmsis_nn_dims *input_dims,
    const cmsis_nn_dims *output_dims, const int32_t lhs_offset, const int32_t *bias_data);

arm_cmsis_nn_status __wrap_arm_vector_sum_s4(
    int32_t *vector_sum_buf, const int8_t *weights_s4, const cmsis_nn_dims *input_dims,
    const cmsis_nn_dims *output_dims, const int32_t lhs_offset, const int32_t *bias_data)
{
  uint32_t t0 = tic_us();
  arm_cmsis_nn_status rc =
      __real_arm_vector_sum_s4(vector_sum_buf, weights_s4, input_dims, output_dims, lhs_offset, bias_data);
  log_kernel("arm_vector_sum_s4", toc_us(t0));
  return rc;
}

// arm_vector_sum_s8
arm_cmsis_nn_status __real_arm_vector_sum_s8(
    int32_t *vector_sum_buf, const int32_t vector_cols, const int32_t vector_rows, const int8_t *vector_data,
    const int32_t lhs_offset, const int32_t rhs_offset, const int32_t *bias_data);

arm_cmsis_nn_status __wrap_arm_vector_sum_s8(
    int32_t *vector_sum_buf, const int32_t vector_cols, const int32_t vector_rows, const int8_t *vector_data,
    const int32_t lhs_offset, const int32_t rhs_offset, const int32_t *bias_data)
{
  uint32_t t0 = tic_us();
    capture_start_counters();
  arm_cmsis_nn_status rc = __real_arm_vector_sum_s8(
      vector_sum_buf, vector_cols, vector_rows, vector_data, lhs_offset, rhs_offset, bias_data);
  capture_end_counters_and_log("arm_vector_sum_s8", toc_us(t0), rc);
  log_kernel("arm_vector_sum_s8", toc_us(t0));
  return rc;
}
