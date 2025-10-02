WRAP_KERNELS := \
	arm_add_s16 \
	arm_add_s8 \
	arm_avgpool_s16 \
	arm_avgpool_s8 \
	arm_batch_matmul_s16 \
	arm_batch_matmul_s8 \
	arm_convolve_1_x_n_s4 \
	arm_convolve_1_x_n_s8 \
	arm_convolve_1x1_s4 \
	arm_convolve_1x1_s4_fast \
	arm_convolve_1x1_s8 \
	arm_convolve_1x1_s8_fast \
	arm_convolve_s16 \
	arm_convolve_s4 \
	arm_convolve_s8 \
	arm_convolve_weight_sum \
	arm_convolve_weight_sum_s4 \
	arm_convolve_wrapper_s16 \
	arm_convolve_wrapper_s4 \
	arm_convolve_wrapper_s8 \
	arm_depthwise_conv_3x3_s8 \
	arm_depthwise_conv_fast_s16 \
	arm_depthwise_conv_s16 \
	arm_depthwise_conv_s4 \
	arm_depthwise_conv_s4_opt \
	arm_depthwise_conv_s8 \
	arm_depthwise_conv_s8_opt \
	arm_depthwise_conv_wrapper_s16 \
	arm_depthwise_conv_wrapper_s4 \
	arm_depthwise_conv_wrapper_s8 \
	arm_depthwise_convolve_weight_sum \
	arm_depthwise_weight_sum_s4 \
	arm_elementwise_add_s16 \
	arm_elementwise_add_s8 \
	arm_elementwise_mul_s16 \
	arm_elementwise_mul_s8 \
	arm_fully_connected_per_channel_s8 \
	arm_fully_connected_s16 \
	arm_fully_connected_s4 \
	arm_fully_connected_s8 \
	arm_fully_connected_wrapper_s8 \
	arm_hard_swish_compat_s16 \
	arm_hard_swish_compat_s8 \
	arm_hard_swish_precise_s16 \
	arm_hard_swish_precise_s8 \
	arm_leaky_relu_s8 \
	arm_lstm_unidirectional_s16 \
	arm_lstm_unidirectional_s8 \
	arm_max_pool_s16 \
	arm_max_pool_s8 \
	arm_maximum_s16 \
	arm_maximum_s8 \
	arm_mean_s16 \
	arm_mean_s8 \
	arm_minimum_s16 \
	arm_minimum_s8 \
	arm_mul_s16 \
	arm_mul_s8 \
	arm_pad_s16 \
	arm_pad_s8 \
	arm_quantize_f32_s16 \
	arm_quantize_f32_s8 \
	arm_reduce_max_s16 \
	arm_reduce_max_s8 \
	arm_relu_s16 \
	arm_relu_s8 \
	arm_requantize_s16_s16 \
	arm_requantize_s8_s8 \
	arm_softmax_s16 \
	arm_softmax_s8 \
	arm_softmax_s8_s16 \
	arm_strided_slice_s8 \
	arm_svdf_s8 \
	arm_svdf_state_s16_s8 \
	arm_transpose_conv_s8 \
	arm_transpose_conv_wrapper_s8 \
	arm_transpose_s16 \
	arm_transpose_s8 \
	arm_vector_sum_s4 \
	arm_vector_sum_s8


LFLAGS += $(foreach S,$(WRAP_KERNELS),-Wl,--wrap=$(S))
LFLAGS += -Wl,-Map,$(BINDIR)/link.map



ifeq ($(TOOLCHAIN),arm-none-eabi)
LFLAGS += -Wl,--wrap=malloc -Wl,--wrap=free
endif