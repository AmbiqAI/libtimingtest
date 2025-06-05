#ifndef aot_model_h
#define aot_model_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define aot_num_inputs 1
#define aot_num_outputs 1

#define aot_input_0_size 27648
#define aot_output_0_size 2

extern const int32_t aot_inputs_len[1];
extern const int32_t aot_inputs_zero_point[1];
extern const float aot_inputs_scale[1];
extern const int32_t aot_outputs_len[1];
extern const int32_t aot_outputs_zero_point[1];
extern const float aot_outputs_scale[1];


/// Operator states passed to the callback
typedef enum {
    aot_model_state_started = 0,  // <-- operator started
    aot_model_state_finished,     // <-- operator finished
} aot_operator_state_e;

/// Signature for the operator callback
typedef void (*aot_operator_callback)(
    int32_t              op,           // <-- operator identifier
    aot_operator_state_e state,  // <-- operator state
    int32_t              status,       // <-- return code
    void                *user_data     // <-- opaque user data
);

/// Holds the callback and opaque user context
typedef struct {
    const void* input_data[1];
    int32_t input_len[1];

    void* output_data[1];
    int32_t output_len[1];

    aot_operator_callback callback;
    void *user_data;
} aot_model_context_t;


int32_t aot_model_init(aot_model_context_t *context);

int32_t aot_model_run(aot_model_context_t *context);

#ifdef __cplusplus
}
#endif

#endif // aot_model_h
