#include <stdint.h>
#include <string.h>
#include "arm_nnfunctions.h"  // CMSIS-NN API
#include "model_ad.h"

// Maximum activation dimension for our sequential model
#define MAX_ACTIVATION_SIZE 640

//---------------------------------------------------------------------
// Activation buffers (ping–pong for intermediate results)
static int8_t act_buffer0[MAX_ACTIVATION_SIZE];
static int8_t act_buffer1[MAX_ACTIVATION_SIZE];

//---------------------------------------------------------------------
// Placeholders for weights and biases (all zero for now)

// Layer 1: FullyConnected 128x640, bias 128, ReLU
static int8_t fc1_weights[128 * 640] = {0};
static int32_t fc1_bias[128] = {0};

// Layer 2: FullyConnected 128x128, bias 128, ReLU
static int8_t fc2_weights[128 * 128] = {0};
static int32_t fc2_bias[128] = {0};

// Layer 3: FullyConnected 128x128, bias 128, ReLU
static int8_t fc3_weights[128 * 128] = {0};
static int32_t fc3_bias[128] = {0};

// Layer 4: FullyConnected 128x128, bias 128, ReLU
static int8_t fc4_weights[128 * 128] = {0};
static int32_t fc4_bias[128] = {0};

// Layer 5: FullyConnected 128x128, bias 128, ReLU
static int8_t fc5_weights[128 * 128] = {0};
static int32_t fc5_bias[128] = {0};

// Layer 6: FullyConnected 8x128, bias 8, ReLU
static int8_t fc6_weights[8 * 128] = {0};
static int32_t fc6_bias[8] = {0};

// Layer 7: FullyConnected 128x8, bias 128, ReLU
static int8_t fc7_weights[128 * 8] = {0};
static int32_t fc7_bias[128] = {0};

// Layer 8: FullyConnected 128x128, bias 128, ReLU
static int8_t fc8_weights[128 * 128] = {0};
static int32_t fc8_bias[128] = {0};

// Layer 9: FullyConnected 128x128, bias 128, ReLU
static int8_t fc9_weights[128 * 128] = {0};
static int32_t fc9_bias[128] = {0};

// Layer 10: FullyConnected 128x128, bias 128, ReLU
static int8_t fc10_weights[128 * 128] = {0};
static int32_t fc10_bias[128] = {0};

// Layer 11: FullyConnected 640x128, bias 640, no activation
static int8_t fc11_weights[640 * 128] = {0};
static int32_t fc11_bias[640] = {0};

static int8_t ctx_buffer[1024];

// Quantization parameter structures for each layer
static cmsis_nn_per_tensor_quant_params quant_params_fc1 = {.multiplier = 0, .shift = 1};
static cmsis_nn_per_tensor_quant_params quant_params_fc2 = {.multiplier = 0, .shift = 1};
static cmsis_nn_per_tensor_quant_params quant_params_fc3 = {.multiplier = 0, .shift = 1};
static cmsis_nn_per_tensor_quant_params quant_params_fc4 = {.multiplier = 0, .shift = 1};
static cmsis_nn_per_tensor_quant_params quant_params_fc5 = {.multiplier = 0, .shift = 1};
static cmsis_nn_per_tensor_quant_params quant_params_fc6 = {.multiplier = 0, .shift = 1};
static cmsis_nn_per_tensor_quant_params quant_params_fc7 = {.multiplier = 0, .shift = 1};
static cmsis_nn_per_tensor_quant_params quant_params_fc8 = {.multiplier = 0, .shift = 1};
static cmsis_nn_per_tensor_quant_params quant_params_fc9 = {.multiplier = 0, .shift = 1};
static cmsis_nn_per_tensor_quant_params quant_params_fc10 = {.multiplier = 0, .shift = 1};
static cmsis_nn_per_tensor_quant_params quant_params_fc11 = {.multiplier = 0, .shift = 1};

//---------------------------------------------------------------------
// Define dimensions for the sequential model.
// We follow the convention for fully connected layers:
//    input:  { n=1, h=1, w=1, c = input_size }
//    filter: { n=1, h=1, w = input_size, c = output_size }
//    bias:   { n=0, h=0, w=0, c = output_size }
//    output: { n=1, h=1, w=1, c = output_size }

// Input to the network:
static const cmsis_nn_dims input_dims = { .n = 1, .h = 0, .w = 0, .c = 640 };

// Layer 1 dims: 640 -> 128
static const cmsis_nn_dims fc1_filter_dims = { .n = 640, .h = 0, .w = 0, .c = 128 };
static const cmsis_nn_dims fc1_bias_dims   = { .n = 0, .h = 0, .w = 0, .c = 128 };
static const cmsis_nn_dims fc1_output_dims = { .n = 1, .h = 0, .w = 0, .c = 128 };

// Layers 2-5 and 8-10: 128 -> 128
static const cmsis_nn_dims fc128_filter_dims = { .n = 128, .h = 0, .w = 0, .c = 128 };
static const cmsis_nn_dims fc128_bias_dims   = { .n = 0, .h = 0, .w = 0, .c = 128 };
static const cmsis_nn_dims fc128_output_dims = { .n = 1, .h = 0, .w = 0, .c = 128 };

// Layer 6: 128 -> 8
static const cmsis_nn_dims fc6_filter_dims = { .n = 128, .h = 0, .w = 0, .c = 8 };
static const cmsis_nn_dims fc6_bias_dims   = { .n = 0, .h = 0, .w = 0, .c = 8 };
static const cmsis_nn_dims fc6_output_dims = { .n = 1, .h = 0, .w = 0, .c = 8 };

// Layer 7: 8 -> 128
static const cmsis_nn_dims fc7_filter_dims = { .n = 8, .h = 0, .w = 0, .c = 128 };
static const cmsis_nn_dims fc7_bias_dims   = { .n = 0, .h = 0, .w = 0, .c = 128 };
static const cmsis_nn_dims fc7_output_dims = { .n = 1, .h = 0, .w = 0, .c = 128 };

// Layer 11: 128 -> 640
static const cmsis_nn_dims fc11_filter_dims = { .n = 128, .h = 1, .w = 128, .c = 640 };
static const cmsis_nn_dims fc11_bias_dims   = { .n = 0, .h = 0, .w = 0, .c = 640 };
static const cmsis_nn_dims fc11_output_dims = { .n = 1, .h = 0, .w = 0, .c = 640 };

//---------------------------------------------------------------------
// FC layer parameter structures
// For layers with ReLU activation, we set activation.min=0 and activation.max=127.
// For the final (linear) layer we use the full int8 range.
static const cmsis_nn_fc_params fc_params_relu = {
    .input_offset  = 0,
    .filter_offset = 0,
    .output_offset = 0,
    .activation    = { .min = 0, .max = 127 }
};

static const cmsis_nn_fc_params fc_params_linear = {
    .input_offset  = 0,
    .filter_offset = 0,
    .output_offset = 0,
    .activation    = { .min = -128, .max = 127 }
};


//---------------------------------------------------------------------
// Model initialization function.
// This sets up the per-layer quantization parameters.
// (Weights and biases remain placeholders for now.)
uint32_t model_ad_init(void)
{
    // Randomly set weights and biases for each layer
    for (int i = 0; i < 128 * 640; i++)
        fc1_weights[i] = (int8_t)(i % 128);
    for (int i = 0; i < 128; i++)
        fc1_bias[i] = (int32_t)(i % 128);
    for (int i = 0; i < 128 * 128; i++)
        fc2_weights[i] = (int8_t)(i % 128);
    for (int i = 0; i < 128; i++)
        fc2_bias[i] = (int32_t)(i % 128);
    for (int i = 0; i < 128 * 128; i++)
        fc3_weights[i] = (int8_t)(i % 128);
    for (int i = 0; i < 128; i++)
        fc3_bias[i] = (int32_t)(i % 128);
    for (int i = 0; i < 128 * 128; i++)
        fc4_weights[i] = (int8_t)(i % 128);
    for (int i = 0; i < 128; i++)
        fc4_bias[i] = (int32_t)(i % 128);
    for (int i = 0; i < 128 * 128; i++)
        fc5_weights[i] = (int8_t)(i % 128);
    for (int i = 0; i < 128; i++)
        fc5_bias[i] = (int32_t)(i % 128);
    for (int i = 0; i < 8 * 128; i++)
        fc6_weights[i] = (int8_t)(i % 8);
    for (int i = 0; i < 8; i++)
        fc6_bias[i] = (int32_t)(i % 8);
    for (int i = 0; i < 128 * 8; i++)
        fc7_weights[i] = (int8_t)(i % 128);
    for (int i = 0; i < 128; i++)
        fc7_bias[i] = (int32_t)(i % 128);
    for (int i = 0; i < 128 * 128; i++)
        fc8_weights[i] = (int8_t)(i % 128);
    for (int i = 0; i < 128; i++)
        fc8_bias[i] = (int32_t)(i % 128);
    for (int i = 0; i < 128 * 128; i++)
        fc9_weights[i] = (int8_t)(i % 128);
    for (int i = 0; i < 128; i++)
        fc9_bias[i] = (int32_t)(i % 128);
    for (int i = 0; i < 128 * 128; i++)
        fc10_weights[i] = (int8_t)(i % 128);
    for (int i = 0; i < 128; i++)
        fc10_bias[i] = (int32_t)(i % 128);
    for (int i = 0; i < 640 * 128; i++)
        fc11_weights[i] = (int8_t)(i % 128);
    for (int i = 0; i < 640; i++)
        fc11_bias[i] = (int32_t)(i % 128);

    return 0;
}

//---------------------------------------------------------------------
// Model run function.
// This function expects an input buffer of 640 int8 values and writes
// the final output (640 int8 values) into the provided output buffer.
uint32_t model_ad_run(const int8_t *input, int8_t *output)
{
    arm_cmsis_nn_status status;
    cmsis_nn_context ctx;
    ctx.buf = ctx_buffer;
    ctx.size = sizeof(ctx_buffer);

    // Pointers for ping-ponging between activation buffers.
    // The first layer takes the external input.
    const int8_t *in_ptr = input;
    int8_t *out_ptr = act_buffer0;

    // --- Layer 1: FC 128x640, ReLU ---
    status = arm_fully_connected_s8(&ctx,
                                    &fc_params_relu,
                                    &quant_params_fc1,
                                    &input_dims,
                                    in_ptr,
                                    &fc1_filter_dims,
                                    fc1_weights,
                                    &fc1_bias_dims,
                                    fc1_bias,
                                    &fc1_output_dims,
                                    out_ptr);
    if (status != ARM_CMSIS_NN_SUCCESS)
        return status;

    // For subsequent layers we alternate between the two activation buffers.
    in_ptr = out_ptr;
    out_ptr = act_buffer1;

    // --- Layer 2: FC 128x128, ReLU ---
    {
        cmsis_nn_dims fc2_input_dims = { .n = 1, .h = 1, .w = 1, .c = 128 };
        status = arm_fully_connected_s8(&ctx,
                                        &fc_params_relu,
                                        &quant_params_fc2,
                                        &fc2_input_dims,
                                        in_ptr,
                                        &fc128_filter_dims,
                                        fc2_weights,
                                        &fc128_bias_dims,
                                        fc2_bias,
                                        &fc128_output_dims,
                                        out_ptr);
        if (status != ARM_CMSIS_NN_SUCCESS)
            return status;
    }
    // Swap buffers
    in_ptr = out_ptr;
    out_ptr = act_buffer0;

    // --- Layer 3: FC 128x128, ReLU ---
    {
        cmsis_nn_dims fc3_input_dims = { .n = 1, .h = 1, .w = 1, .c = 128 };
        status = arm_fully_connected_s8(&ctx,
                                        &fc_params_relu,
                                        &quant_params_fc3,
                                        &fc3_input_dims,
                                        in_ptr,
                                        &fc128_filter_dims,  // same dims as fc2 for 128x128
                                        fc3_weights,
                                        &fc128_bias_dims,
                                        fc3_bias,
                                        &fc128_output_dims,
                                        out_ptr);
        if (status != ARM_CMSIS_NN_SUCCESS)
            return status;
    }
    // Swap buffers
    in_ptr = out_ptr;
    out_ptr = act_buffer1;

    // --- Layer 4: FC 128x128, ReLU ---
    {
        cmsis_nn_dims fc4_input_dims = { .n = 1, .h = 1, .w = 1, .c = 128 };
        status = arm_fully_connected_s8(&ctx,
                                        &fc_params_relu,
                                        &quant_params_fc4,
                                        &fc4_input_dims,
                                        in_ptr,
                                        &fc128_filter_dims,
                                        fc4_weights,
                                        &fc128_bias_dims,
                                        fc4_bias,
                                        &fc128_output_dims,
                                        out_ptr);
        if (status != ARM_CMSIS_NN_SUCCESS)
            return status;
    }
    // Swap buffers
    in_ptr = out_ptr;
    out_ptr = act_buffer0;

    // --- Layer 5: FC 128x128, ReLU ---
    {
        cmsis_nn_dims fc5_input_dims = { .n = 1, .h = 1, .w = 1, .c = 128 };
        status = arm_fully_connected_s8(&ctx,
                                        &fc_params_relu,
                                        &quant_params_fc5,
                                        &fc5_input_dims,
                                        in_ptr,
                                        &fc128_filter_dims,
                                        fc5_weights,
                                        &fc128_bias_dims,
                                        fc5_bias,
                                        &fc128_output_dims,
                                        out_ptr);
        if (status != ARM_CMSIS_NN_SUCCESS)
            return status;
    }
    // Swap buffers
    in_ptr = out_ptr;
    out_ptr = act_buffer1;

    // --- Layer 6: FC 8x128, ReLU ---
    {
        cmsis_nn_dims fc6_input_dims = { .n = 1, .h = 1, .w = 1, .c = 128 };
        status = arm_fully_connected_s8(&ctx,
                                        &fc_params_relu,
                                        &quant_params_fc6,
                                        &fc6_input_dims,
                                        in_ptr,
                                        &fc6_filter_dims,
                                        fc6_weights,
                                        &fc6_bias_dims,
                                        fc6_bias,
                                        &fc6_output_dims,
                                        out_ptr);
        if (status != ARM_CMSIS_NN_SUCCESS)
            return status;
    }
    // For the next layer, note that the dimension changes (8 outputs).
    in_ptr = out_ptr;
    // We reuse one of our activation buffers; since 8 < MAX_ACTIVATION_SIZE we are safe.
    out_ptr = act_buffer0;

    // --- Layer 7: FC 128x8, ReLU ---
    {
        cmsis_nn_dims fc7_input_dims = { .n = 1, .h = 1, .w = 1, .c = 8 };
        status = arm_fully_connected_s8(&ctx,
                                        &fc_params_relu,
                                        &quant_params_fc7,
                                        &fc7_input_dims,
                                        in_ptr,
                                        &fc7_filter_dims,
                                        fc7_weights,
                                        &fc7_bias_dims,
                                        fc7_bias,
                                        &fc128_output_dims,
                                        out_ptr);
        if (status != ARM_CMSIS_NN_SUCCESS)
            return status;
    }
    // Swap buffers (back to 128-dim activation)
    in_ptr = out_ptr;
    out_ptr = act_buffer1;

    // --- Layer 8: FC 128x128, ReLU ---
    {
        cmsis_nn_dims fc8_input_dims = { .n = 1, .h = 1, .w = 1, .c = 128 };
        status = arm_fully_connected_s8(&ctx,
                                        &fc_params_relu,
                                        &quant_params_fc8,
                                        &fc8_input_dims,
                                        in_ptr,
                                        &fc128_filter_dims,
                                        fc8_weights,
                                        &fc128_bias_dims,
                                        fc8_bias,
                                        &fc128_output_dims,
                                        out_ptr);
        if (status != ARM_CMSIS_NN_SUCCESS)
            return status;
    }
    // Swap buffers
    in_ptr = out_ptr;
    out_ptr = act_buffer0;

    // --- Layer 9: FC 128x128, ReLU ---
    {
        cmsis_nn_dims fc9_input_dims = { .n = 1, .h = 1, .w = 1, .c = 128 };
        status = arm_fully_connected_s8(&ctx,
                                        &fc_params_relu,
                                        &quant_params_fc9,
                                        &fc9_input_dims,
                                        in_ptr,
                                        &fc128_filter_dims,
                                        fc9_weights,
                                        &fc128_bias_dims,
                                        fc9_bias,
                                        &fc128_output_dims,
                                        out_ptr);
        if (status != ARM_CMSIS_NN_SUCCESS)
            return status;
    }
    // Swap buffers
    in_ptr = out_ptr;
    out_ptr = act_buffer1;

    // --- Layer 10: FC 128x128, ReLU ---
    {
        cmsis_nn_dims fc10_input_dims = { .n = 1, .h = 1, .w = 1, .c = 128 };
        status = arm_fully_connected_s8(&ctx,
                                        &fc_params_relu,
                                        &quant_params_fc10,
                                        &fc10_input_dims,
                                        in_ptr,
                                        &fc128_filter_dims,
                                        fc10_weights,
                                        &fc128_bias_dims,
                                        fc10_bias,
                                        &fc128_output_dims,
                                        out_ptr);
        if (status != ARM_CMSIS_NN_SUCCESS)
            return status;
    }
    // Swap buffers for final layer.
    in_ptr = out_ptr;
    out_ptr = act_buffer0;

    // --- Layer 11: FC 640x128, no activation ---
    {
        cmsis_nn_dims fc11_input_dims = { .n = 1, .h = 1, .w = 1, .c = 128 };
        status = arm_fully_connected_s8(&ctx,
                                        &fc_params_linear,
                                        &quant_params_fc11,
                                        &fc11_input_dims,
                                        in_ptr,
                                        &fc11_filter_dims,
                                        fc11_weights,
                                        &fc11_bias_dims,
                                        fc11_bias,
                                        &fc11_output_dims,
                                        out_ptr);
        if (status != ARM_CMSIS_NN_SUCCESS)
            return status;
    }

    // Final output (640 int8 values) is now in out_ptr.
    memcpy(output, out_ptr, 640 * sizeof(int8_t));

    return ARM_CMSIS_NN_SUCCESS;
}
