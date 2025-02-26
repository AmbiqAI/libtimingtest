// neuralSPOT
#include "ns_ambiqsuite_harness.h"
// TFLM
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
#include "tensorflow/lite/micro/cortex_m_generic/debug_log_callback.h"
#include "tensorflow/lite/schema/schema_generated.h"
// Locals
#include <arm_fp16.h>
#include "tflm.h"
// #include "model_flatbuffer_fp32.h"
// #include "model_flatbuffer_fp16.h"
// #include "model_flatbuffer_s8.h"
#include "model_ad_flatbuffer.h"
// #include "model_flatbuffer_s16.h"
// #include "nnse_16x8_flatbuffer.h"
// #include "nnse_int16.h"
#include "model.h"

#define MODEL_ARENA_SIZE_KB (800)
#define MODEL_RESOURCE_VAR_COUNT (4)
constexpr int kNumResourceVariables = 24;
static constexpr int modelTensorArenaSize = 1024 * MODEL_ARENA_SIZE_KB;
static constexpr int modelResourceVarArenaSize = 4 * (1 + MODEL_RESOURCE_VAR_COUNT) * sizeof(tflite::MicroResourceVariables);
alignas(16) static uint8_t model_var_arena[modelResourceVarArenaSize];
AM_SHARED_RW alignas(16) static uint8_t modelTensorArena[modelTensorArenaSize];
tf_model_context_t modelCtx = {
    .arenaSize = modelTensorArenaSize,
    .arena = modelTensorArena,
    .buffer = model_ad_flatbuffer,
    .model = nullptr,
    .input = nullptr,
    .output = nullptr,
    .interpreter = nullptr,
};


extern ns_timer_config_t timerCfg;

void
model_TFDebugLog(const char *s) {
    ns_printf("%s", s);
}

uint32_t
model_init() {

    size_t bytesUsed;
    TfLiteStatus allocateStatus;
    tf_model_context_t *ctx = &modelCtx;

    RegisterDebugLogCallback(model_TFDebugLog);

    // Initialize TFLM backend
    tflm_init_model(ctx);

    // Load model
    ctx->model = tflite::GetModel(ctx->buffer);
    if (ctx->model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(ctx->reporter, "Schema mismatch: given=%d != expected=%d.", ctx->model->version(), TFLITE_SCHEMA_VERSION);
        return 1;
    }
    // Initialize interpreter
    if (ctx->interpreter != nullptr) { ctx->interpreter->Reset();}

    tflite::MicroAllocator* allocator(
        tflite::MicroAllocator::Create(ctx->arena, ctx->arenaSize));


    static tflite::MicroInterpreter model_interpreter(
        ctx->model,
        *(ctx->resolver),
        allocator,
        tflite::MicroResourceVariables::Create(allocator, kNumResourceVariables),
        ctx->profiler
    );
    ctx->interpreter = &model_interpreter;

    // Allocate tensors
    allocateStatus = ctx->interpreter->AllocateTensors();
    if (allocateStatus != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(ctx->reporter, "AllocateTensors() failed");
        ns_printf("[MODEL] AllocateTensors() failed\n");
        return 1;
    }

    // Check arena size
    bytesUsed = ctx->interpreter->arena_used_bytes();
    ns_lp_printf("[MODEL] Arena used: %d bytes\n", bytesUsed);
    if (bytesUsed > ctx->arenaSize) {
        ns_printf("[MODEL] Arena mismatch\n");
        TF_LITE_REPORT_ERROR(ctx->reporter, "Arena mismatch: given=%d < expected=%d bytes.", ctx->arenaSize, bytesUsed);
        return 1;
    }

    // Store input and output pointers (assume single input/output tensor)
    ctx->input = ctx->interpreter->input(0);
    ctx->output = ctx->interpreter->output(0);
    return 0;
}

uint32_t
model_inference() {
    float32_t yVal, yMax = 0;
    float16_t val_fp16 = 0;
    uint32_t yMaxIdx = 0;
    tf_model_context_t *ctx = &modelCtx;
    uint32_t ticUs = 0, tocUs = 0;

    int num_elements = 1;
    for (int i = 0; i < ctx->input->dims->size; ++i) {
        num_elements *= ctx->input->dims->data[i];
    }
    // ns_lp_printf("[MODEL] Input tensor size: %d (type: %d)\n", num_elements, ctx->input->type);
    for (int i = 0; i < num_elements; i++) {
        if (ctx->input->quantization.type == kTfLiteAffineQuantization) {
            if (ctx->input->type == kTfLiteInt8) {
                ctx->input->data.int8[i] = (int8_t)i%256;
            } else {
                ctx->input->data.i16[i] = (int16_t)(i%256);
            }
        } else if (ctx->input->type == kTfLiteFloat32) {
            ctx->input->data.f[i] = (float32_t)(i%256);
        } else if (ctx->input->type == kTfLiteFloat16) {
            val_fp16 = (float16_t)(i%256);
            memcpy(&ctx->input->data.f16[i].data, &val_fp16, sizeof(float16_t));
        }
    }

    // Invoke model
    ns_timer_clear(&timerCfg);
    ticUs = ns_us_ticker_read(&timerCfg);
    TfLiteStatus invokeStatus = ctx->interpreter->Invoke();
    tocUs = ns_us_ticker_read(&timerCfg);
    ns_lp_printf("[MODEL] Inference time (%d): %d us\n", tocUs - ticUs, invokeStatus);

    // ctx->profiler->LogCsv();

    // Print output
    num_elements = 1;
    for (int i = 0; i < ctx->output->dims->size; ++i) {
        num_elements *= ctx->output->dims->data[i];
    }

    // ns_lp_printf("[S8 MODEL] Output tensor size: %d (type: %d)\n", num_elements, ctx->output->type);
    for (int i = 0; i < num_elements; i++) {
        if (ctx->output->quantization.type == kTfLiteAffineQuantization) {
            if (ctx->output->type == kTfLiteInt8) {
                yVal = (float32_t)ctx->output->data.int8[i];
            } else {
                yVal = (float32_t)ctx->output->data.i16[i];
            }
        } else if (ctx->output->type == kTfLiteFloat32) {
            yVal = ctx->output->data.f[i];
        } else if (ctx->output->type == kTfLiteFloat16) {
            memcpy(&val_fp16, &ctx->output->data.f16[i].data, sizeof(float16_t));
            yVal = (float32_t)val_fp16;
        }
        // ns_lp_printf("[ MODEL] Output[%d]: %f\n", i, yVal);
    }

    return invokeStatus;
}
