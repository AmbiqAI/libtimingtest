#ifndef TEST_LIBRARY_H
#define TEST_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif

// Test lists organized by category

// Convolution tests
#define CONVOLUTION_TEST_LIST \
  X(basic_2_arm_convolve_s4) \
  X(basic_arm_convolve_s4) \
  X(basic_arm_convolve_s8) \
  X(basic_arm_depthwise_conv_s8_opt) \
  X(buffer_size_arm_convolve_1x1_s4_fast) \
  X(buffer_size_arm_convolve_1x1_s8_fast) \
  X(buffer_size_arm_convolve_s16) \
  X(buffer_size_arm_convolve_s4) \
  X(buffer_size_arm_convolve_s8) \
  X(buffer_size_arm_depthwise_conv_fast_s16) \
  X(buffer_size_arm_depthwise_conv_s8_opt) \
  X(buffer_size_dsp_arm_convolve_1x1_s4_fast) \
  X(buffer_size_dsp_arm_convolve_1x1_s8_fast) \
  X(buffer_size_dsp_arm_convolve_s16) \
  X(buffer_size_dsp_arm_convolve_s4) \
  X(buffer_size_dsp_arm_convolve_s8) \
  X(buffer_size_dsp_arm_depthwise_conv_3x3_s8) \
  X(buffer_size_dsp_arm_depthwise_conv_fast_s16) \
  X(buffer_size_dsp_arm_depthwise_conv_s16) \
  X(buffer_size_dsp_arm_depthwise_conv_s8) \
  X(buffer_size_dsp_arm_depthwise_conv_s8_opt) \
  X(buffer_size_mve_arm_convolve_1x1_s4_fast) \
  X(buffer_size_mve_arm_convolve_1x1_s8_fast) \
  X(buffer_size_mve_arm_convolve_s16) \
  X(buffer_size_mve_arm_convolve_s4) \
  X(buffer_size_mve_arm_convolve_s8) \
  X(buffer_size_mve_arm_depthwise_conv_3x3_s8) \
  X(buffer_size_mve_arm_depthwise_conv_fast_s16) \
  X(buffer_size_mve_arm_depthwise_conv_s16) \
  X(buffer_size_mve_arm_depthwise_conv_s8) \
  X(buffer_size_mve_arm_depthwise_conv_s8_opt) \
  X(conv_1_x_n_1_arm_convolve_s4) \
  X(conv_1_x_n_1_arm_convolve_s8) \
  X(conv_1_x_n_2_arm_convolve_s4) \
  X(conv_1_x_n_2_arm_convolve_s8) \
  X(conv_1_x_n_3_arm_convolve_s4) \
  X(conv_1_x_n_3_arm_convolve_s8) \
  X(conv_1_x_n_4_arm_convolve_s4) \
  X(conv_1_x_n_4_arm_convolve_s8) \
  X(conv_1_x_n_5_arm_convolve_s4) \
  X(conv_1_x_n_5_arm_convolve_s8) \
  X(conv_1_x_n_6_arm_convolve_s8) \
  X(conv_1_x_n_6_generic_arm_convolve_s8) \
  X(conv_1_x_n_7_arm_convolve_s8) \
  X(conv_1_x_n_8_arm_convolve_s8) \
  X(conv_2_arm_convolve_s4) \
  X(conv_2_arm_convolve_s8) \
  X(conv_2x2_dilation_5x5_input_arm_convolve_s4) \
  X(conv_2x2_dilation_5x5_input_arm_convolve_s8) \
  X(conv_2x2_dilation_arm_convolve_s4) \
  X(conv_2x2_dilation_arm_convolve_s8) \
  X(conv_2x3_dilation_arm_convolve_s4) \
  X(conv_2x3_dilation_arm_convolve_s8) \
  X(conv_3_arm_convolve_s4) \
  X(conv_3_arm_convolve_s8) \
  X(conv_3x2_dilation_arm_convolve_s4) \
  X(conv_3x2_dilation_arm_convolve_s8) \
  X(conv_3x3_dilation_5x5_input_arm_convolve_s4) \
  X(conv_3x3_dilation_5x5_input_arm_convolve_s8) \
  X(conv_4_arm_convolve_s4) \
  X(conv_4_arm_convolve_s8) \
  X(conv_5_arm_convolve_s4) \
  X(conv_5_arm_convolve_s8) \
  X(conv_dilation_golden_arm_convolve_s4) \
  X(conv_dilation_golden_arm_convolve_s8) \
  X(conv_out_activation_arm_convolve_s4) \
  X(conv_out_activation_arm_convolve_s8) \
  X(conv_refactored_fc_conv_dilated) \
  X(conv_refactored_fc_conv_int8_1x1_kernel) \
  X(conv_refactored_fc_conv_int8_diff_channels) \
  X(conv_refactored_fc_conv_int8_non_4_multiple) \
  X(depthwise_2_arm_depthwise_conv_s8) \
  X(depthwise_dilation_arm_depthwise_conv_s8) \
  X(depthwise_eq_in_out_ch_arm_depthwise_conv_s8_opt) \
  X(depthwise_int4_1_arm_depthwise_conv_s4_opt) \
  X(depthwise_int4_2_arm_depthwise_conv_s4_opt) \
  X(depthwise_int4_3_arm_depthwise_conv_s4_opt) \
  X(depthwise_int4_4_arm_depthwise_conv_s4_opt) \
  X(depthwise_int4_generic_2_arm_depthwise_conv_s4) \
  X(depthwise_int4_generic_3_arm_depthwise_conv_s4) \
  X(depthwise_int4_generic_4_arm_depthwise_conv_s4) \
  X(depthwise_int4_generic_5_arm_depthwise_conv_s4) \
  X(depthwise_int4_generic_6_arm_depthwise_conv_s4) \
  X(depthwise_int4_generic_arm_depthwise_conv_s4) \
  X(depthwise_kernel_3x3_arm_depthwise_conv_3x3_1_s8) \
  X(depthwise_kernel_3x3_arm_depthwise_conv_3x3_s8) \
  X(depthwise_kernel_3x3_null_bias_arm_depthwise_conv_3x3_null_bias_s8) \
  X(depthwise_mult_batches_arm_depthwise_conv_s8) \
  X(depthwise_null_bias_0_arm_depthwise_conv_s8_opt) \
  X(depthwise_null_bias_1_arm_depthwise_conv_s8) \
  X(depthwise_out_activation_arm_depthwise_conv_s8_opt) \
  X(depthwise_sub_block_arm_depthwise_conv_s8_opt) \
  X(depthwise_x_stride_arm_depthwise_conv_s8_opt) \
  X(dw_int16xint8_arm_depthwise_conv_s16) \
  X(dw_int16xint8_dilation_arm_depthwise_conv_s16) \
  X(dw_int16xint8_fast_arm_depthwise_conv_fast_s16) \
  X(dw_int16xint8_fast_multiple_batches_uneven_buffers_arm_depthwise_conv_fast_s16) \
  X(dw_int16xint8_fast_multiple_batches_uneven_buffers_null_bias_arm_depthwise_conv_fast_s16) \
  X(dw_int16xint8_fast_null_bias_arm_depthwise_conv_fast_s16) \
  X(dw_int16xint8_fast_spill_arm_depthwise_conv_fast_s16) \
  X(dw_int16xint8_fast_spill_null_bias_arm_depthwise_conv_fast_s16) \
  X(dw_int16xint8_fast_stride_arm_depthwise_conv_fast_s16) \
  X(dw_int16xint8_fast_stride_null_bias_arm_depthwise_conv_fast_s16) \
  X(dw_int16xint8_fast_test_bias_arm_depthwise_conv_fast_s16) \
  X(dw_int16xint8_mult4_arm_depthwise_conv_s16) \
  X(grouped_conv_arm_grouped_convolve_1_s8) \
  X(grouped_conv_arm_grouped_convolve_2_s8) \
  X(grouped_conv_arm_grouped_convolve_3_s8) \
  X(grouped_conv_arm_grouped_convolve_4_s8) \
  X(in_ch_one_out_ch_larger_one_arm_depthwise_conv_s8) \
  X(int16xint8_1x1_ns_np_nd_arm_convolve_s16) \
  X(int16xint8_arm_convolve_s16) \
  X(int16xint8_dilation_1_arm_convolve_s16) \
  X(int16xint8_dilation_2_arm_convolve_s16) \
  X(int16xint8_dilation_3_arm_convolve_s16) \
  X(int16xint8_group2_arm_convolve_s16) \
  X(int16xint8_group_depthwise_arm_convolve_s16) \
  X(int16xint8_kernel_less_than_9_arm_convolve_s16) \
  X(int16xint8_spill2_arm_convolve_s16) \
  X(int16xint8_spill_arm_convolve_s16) \
  X(int16xint8xint32_1_arm_convolve_s16) \
  X(int16xint8xint32_2_arm_convolve_s16) \
  X(int16xint8xint32_3_arm_convolve_s16) \
  X(int16xint8xint32_4_arm_convolve_s16) \
  X(int16xint8xint32_5_arm_convolve_s16) \
  X(int16xint8xint32_6_arm_convolve_s16) \
  X(kernel1x1_2_arm_convolve_1x1_s4_fast) \
  X(kernel1x1_3_arm_convolve_1x1_s4_fast) \
  X(kernel1x1_arm_convolve_1x1_s4_fast) \
  X(kernel1x1_arm_convolve_1x1_s8_fast) \
  X(kernel1x1_stride_x_arm_convolve_1x1_s4) \
  X(kernel1x1_stride_x_arm_convolve_1x1_s8) \
  X(kernel1x1_stride_x_y_1_arm_convolve_1x1_s4) \
  X(kernel1x1_stride_x_y_1_arm_convolve_1x1_s8) \
  X(kernel1x1_stride_x_y_2_arm_convolve_1x1_s4) \
  X(kernel1x1_stride_x_y_arm_convolve_1x1_s4) \
  X(requantize_s64_arm_convolve_s16) \
  X(simple_dconv_no_bias) \
  X(stride2pad1_arm_convolve_s4) \
  X(stride2pad1_arm_convolve_s8) \
  X(stride2pad1_arm_depthwise_conv_3x3_s8) \
  X(transpose_conv_1_arm_transpose_conv_s8) \
  X(transpose_conv_2_arm_transpose_conv_s8) \
  X(transpose_conv_3_arm_transpose_conv_s8) \
  X(transpose_conv_4_arm_transpose_conv_s8) \
  X(reverse_transpose_conv_1_arm_transpose_conv_s8) \
  X(reverse_transpose_conv_2_arm_transpose_conv_s8) \
  X(reverse_transpose_conv_3_arm_transpose_conv_s8) \
  X(reverse_transpose_conv_4_arm_transpose_conv_s8) \
  X(arm_depthwise_conv_wrapper_s16_buffer)

// Pooling tests
#define POOLING_TEST_LIST \
  X(avgpooling_1_arm_avgpool_s8) \
  X(avgpooling_2_arm_avgpool_s8) \
  X(avgpooling_3_arm_avgpool_s8) \
  X(avgpooling_4_arm_avgpool_s8) \
  X(avgpooling_5_arm_avgpool_s8) \
  X(avgpooling_arm_avgpool_s8) \
  X(avgpooling_int16_1_arm_avgpool_s16) \
  X(avgpooling_int16_2_arm_avgpool_s16) \
  X(avgpooling_int16_3_arm_avgpool_s16) \
  X(avgpooling_int16_arm_avgpool_s16) \
  X(avgpooling_int16_param_fail_arm_avgpool_s16) \
  X(avgpooling_param_fail_arm_avgpool_s8) \
  X(buffer_size_dsp_arm_avgpool_s16) \
  X(buffer_size_dsp_arm_avgpool_s8) \
  X(buffer_size_mve_arm_avgpool_s16) \
  X(buffer_size_mve_arm_avgpool_s8) \
  X(maxpool_int16_1_arm_max_pool_s16) \
  X(maxpool_int16_2_arm_max_pool_s16) \
  X(maxpool_int16_arm_max_pool_s16) \
  X(maxpool_int16_param_fail_arm_max_pool_s16) \
  X(maxpooling_1_arm_max_pool_s8) \
  X(maxpooling_2_arm_max_pool_s8) \
  X(maxpooling_3_arm_max_pool_s8) \
  X(maxpooling_4_arm_max_pool_s8) \
  X(maxpooling_5_arm_max_pool_s8) \
  X(maxpooling_6_arm_max_pool_s8) \
  X(maxpooling_7_arm_max_pool_s8) \
  X(maxpooling_arm_max_pool_s8) \
  X(maxpooling_param_fail_arm_max_pool_s8)

// Arithmetic tests
#define ARITHMETIC_TEST_LIST \
  X(add_arm_elementwise_add_s8) \
  X(add_broadcast_c_s16_arm_add_s16) \
  X(add_broadcast_c_s8_arm_add_s8) \
  X(add_broadcast_h_s16_arm_add_s16) \
  X(add_broadcast_h_s8_arm_add_s8) \
  X(add_broadcast_hc_s16_arm_add_s16) \
  X(add_broadcast_hc_s8_arm_add_s8) \
  X(add_broadcast_w_s16_arm_add_s16) \
  X(add_broadcast_w_s8_arm_add_s8) \
  X(add_ident_s16_arm_add_s16) \
  X(add_ident_s8_arm_add_s8) \
  X(add_s16_arm_elementwise_add_s16) \
  X(add_s16_spill_arm_elementwise_add_s16) \
  X(add_scalar_s16_arm_add_s16) \
  X(add_scalar_s8_arm_add_s8) \
  X(maximum_broadcast_batch_int16) \
  X(maximum_broadcast_batch_int8) \
  X(maximum_broadcast_ch_int16) \
  X(maximum_broadcast_ch_int8) \
  X(maximum_broadcast_height_int16) \
  X(maximum_broadcast_height_int8) \
  X(maximum_broadcast_width_int16) \
  X(maximum_broadcast_width_int8) \
  X(maximum_no_broadcast_int16) \
  X(maximum_no_broadcast_int8) \
  X(maximum_scalar_1_int16) \
  X(maximum_scalar_1_int8) \
  X(maximum_scalar_2_int16) \
  X(maximum_scalar_2_int8) \
  X(minimum_broadcast_batch_int16) \
  X(minimum_broadcast_batch_int8) \
  X(minimum_broadcast_ch_int16) \
  X(minimum_broadcast_ch_int8) \
  X(minimum_broadcast_height_int16) \
  X(minimum_broadcast_height_int8) \
  X(minimum_broadcast_width_int16) \
  X(minimum_broadcast_width_int8) \
  X(minimum_no_broadcast_int16) \
  X(minimum_no_broadcast_int8) \
  X(minimum_scalar_1_int16) \
  X(minimum_scalar_1_int8) \
  X(minimum_scalar_2_int16) \
  X(minimum_scalar_2_int8) \
  X(mul_arm_elementwise_mul_1_s8) \
  X(mul_arm_elementwise_mul_2_s8) \
  X(mul_arm_elementwise_mul_s8) \
  X(mul_broadcast_c_s16_arm_mul_s16) \
  X(mul_broadcast_c_s8_arm_mul_s8) \
  X(mul_broadcast_h_s16_arm_mul_s16) \
  X(mul_broadcast_h_s8_arm_mul_s8) \
  X(mul_broadcast_hc_s16_arm_mul_s16) \
  X(mul_broadcast_hc_s8_arm_mul_s8) \
  X(mul_broadcast_w_s16_arm_mul_s16) \
  X(mul_broadcast_w_s8_arm_mul_s8) \
  X(mul_ident_s16_arm_mul_s16) \
  X(mul_ident_s8_arm_mul_s8) \
  X(mul_s16_arm_elementwise_mul_s16) \
  X(mul_s16_spill_arm_elementwise_mul_s16) \
  X(mul_scalar_s16_arm_mul_s16) \
  X(mul_scalar_s8_arm_mul_s8)

// Fully connected tests
#define FULLY_CONNECTED_TEST_LIST \
  X(batch_matmul_1_s16) \
  X(batch_matmul_1_s8) \
  X(batch_matmul_2_s16) \
  X(batch_matmul_2_s8) \
  X(batch_matmul_3_s16) \
  X(batch_matmul_3_s8) \
  X(batch_matmul_4_s16) \
  X(batch_matmul_4_s8) \
  X(batch_matmul_5_s16) \
  X(batch_matmul_5_s8) \
  X(fc_int16_slow_arm_fully_connected_s16) \
  X(fc_per_ch_arm_fully_connected_s8) \
  X(fully_connected_arm_fully_connected_s8) \
  X(fully_connected_int16_arm_fully_connected_s16) \
  X(fully_connected_int16_big_arm_fully_connected_s16) \
  X(fully_connected_int4_arm_fully_connected_s4) \
  X(fully_connected_int4_arm_fully_connected_s4_2) \
  X(fully_connected_int4_arm_fully_connected_s4_3) \
  X(fully_connected_int4_arm_fully_connected_s4_4) \
  X(fully_connected_int4_arm_fully_connected_s4_5) \
  X(fully_connected_int4_arm_fully_connected_s4_6) \
  X(fully_connected_mve_0_arm_fully_connected_s8) \
  X(fully_connected_mve_1_arm_fully_connected_s8) \
  X(fully_connected_null_bias_0_arm_fully_connected_s8) \
  X(fully_connected_out_activation_arm_fully_connected_s8) \
  X(fully_connected_w_zp_arm_fully_connected_s8)

// Activation tests
#define ACTIVATION_TEST_LIST \
  X(leaky_relu_arm_leaky_relu_s8) \
  X(relu_basic_arm_relu_s8) \
  X(softmax_arm_softmax_s8) \
  X(softmax_invalid_diff_min_arm_softmax_s8) \
  X(softmax_s16_arm_softmax_s16) \
  X(softmax_s8_s16_arm_softmax_s8_s16) \
  X(softmax_s8_s16_invalid_diff_min_arm_softmax_s8_s16)

// Quantization tests
#define QUANTIZATION_TEST_LIST \
  X(test_arm_dequantize_s16_f32) \
  X(test_arm_dequantize_s8_f32) \
  X(test_arm_quantize_f32_s16) \
  X(test_arm_quantize_f32_s8) \
  X(test_arm_quantize_s8_s8)

// LSTM tests
#define LSTM_TEST_LIST \
  X(lstm_1) \
  X(lstm_1_s16) \
  X(lstm_2) \
  X(lstm_2_s16) \
  X(lstm_one_time_step) \
  X(lstm_one_time_step_s16)

// Utility tests
#define UTILITY_TEST_LIST \
  X(mean_axis_c_arm_mean_s16) \
  X(mean_axis_c_arm_mean_s8) \
  X(mean_axis_h_arm_mean_s16) \
  X(mean_axis_h_arm_mean_s8) \
  X(mean_axis_hw_arm_mean_s16) \
  X(mean_axis_hw_arm_mean_s8) \
  X(mean_axis_hwc_arm_mean_s16) \
  X(mean_axis_hwc_arm_mean_s8) \
  X(mean_axis_n_arm_mean_s16) \
  X(mean_axis_n_arm_mean_s8) \
  X(mean_axis_nhwc_arm_mean_s16) \
  X(mean_axis_nhwc_arm_mean_s8) \
  X(mean_axis_w_arm_mean_s16) \
  X(mean_axis_w_arm_mean_s8) \
  X(mean_axis_wc_arm_mean_s16) \
  X(mean_axis_wc_arm_mean_s8) \
  X(pad_int16_1_arm_pad_s16) \
  X(pad_int16_2_arm_pad_s16) \
  X(pad_int8_1_arm_pad_s8) \
  X(pad_int8_2_arm_pad_s8) \
  X(reduce_max_axis_c_arm_reduce_max_s16) \
  X(reduce_max_axis_c_arm_reduce_max_s8) \
  X(reduce_max_axis_h_arm_reduce_max_s16) \
  X(reduce_max_axis_h_arm_reduce_max_s8) \
  X(reduce_max_axis_hw_arm_reduce_max_s16) \
  X(reduce_max_axis_hw_arm_reduce_max_s8) \
  X(reduce_max_axis_hwc_arm_reduce_max_s16) \
  X(reduce_max_axis_hwc_arm_reduce_max_s8) \
  X(reduce_max_axis_n_arm_reduce_max_s16) \
  X(reduce_max_axis_n_arm_reduce_max_s8) \
  X(reduce_max_axis_nhwc_arm_reduce_max_s16) \
  X(reduce_max_axis_nhwc_arm_reduce_max_s8) \
  X(reduce_max_axis_w_arm_reduce_max_s16) \
  X(reduce_max_axis_w_arm_reduce_max_s8) \
  X(reduce_max_axis_wc_arm_reduce_max_s16) \
  X(reduce_max_axis_wc_arm_reduce_max_s8) \
  X(slice_case2_arm_strided_slice_s8) \
  X(slice_case3_arm_strided_slice_s8) \
  X(slice_case4_arm_strided_slice_s8) \
  X(slice_center_3x3_arm_strided_slice_s8) \
  X(slice_strided_2x3_arm_strided_slice_s8) \
  X(slice_whole_slab_arm_strided_slice_s8) \
  X(transpose_3dim2_arm_transpose_s16) \
  X(transpose_3dim2_arm_transpose_s8) \
  X(transpose_3dim_arm_transpose_s16) \
  X(transpose_3dim_arm_transpose_s8) \
  X(transpose_chwn_arm_transpose_s16) \
  X(transpose_chwn_arm_transpose_s8) \
  X(transpose_default_arm_transpose_s16) \
  X(transpose_default_arm_transpose_s8) \
  X(transpose_matrix_arm_transpose_s16)

// SVDF tests
#define SVDF_TEST_LIST \
  X(svdf_1_arm_svdf_state_s16_s8) \
  X(svdf_2_arm_svdf_state_s16_s8) \
  X(svdf_3_arm_svdf_state_s16_s8) \
  X(svdf_int8_2_arm_svdf_s8) \
  X(svdf_int8_arm_svdf_s8)

// DS-CNN tests
#define DS_CNN_TEST_LIST \
  X(weight_presum)

// Combined test list
#define TEST_LIST \
  CONVOLUTION_TEST_LIST \
  FULLY_CONNECTED_TEST_LIST \
  // POOLING_TEST_LIST \
  // ARITHMETIC_TEST_LIST \
  // ACTIVATION_TEST_LIST \
  // QUANTIZATION_TEST_LIST \
  // LSTM_TEST_LIST \
  // UTILITY_TEST_LIST \
  // SVDF_TEST_LIST \
  // DS_CNN_TEST_LIST

// Tests that hang so I leave them here for now: 
//   X(ds_cnn_l_s8_inference) \
//   X(ds_cnn_s_s8_inference) \
//   X(transpose_ncwh_arm_transpose_s16) \
//   X(transpose_nchw_arm_transpose_s8) \
//   X(transpose_ncwh_arm_transpose_s8) \
//   X(transpose_nchw_arm_transpose_s16) \
//   X(transpose_matrix_arm_transpose_s8) \
//   X(transpose_nhcw_arm_transpose_s16) \
//   X(transpose_nhcw_arm_transpose_s8) \
//   X(transpose_nwhc_arm_transpose_s16) \
//   X(transpose_nwhc_arm_transpose_s8) \
//   X(transpose_wchn_arm_transpose_s16) \
//   X(transpose_wchn_arm_transpose_s8) \
//   X(single_in_many_out_ch) \
//   X(svdf_arm_svdf_state_s16_s8) \
//   X(kernel1x1_stride_x_y_2_arm_convolve_1x1_s8) \
//   X(kernel1x1_stride_x_y_arm_convolve_1x1_s8) \
// 



void test_library(void);

void test_library_step(unsigned budget);

// True when we've run every test in TEST_LIST.
int  test_library_done(void);

// Reset the internal cursor back to the first test.
void test_library_reset(void);

#ifdef __cplusplus
}
#endif

#endif // TEST_LIBRARY_H
