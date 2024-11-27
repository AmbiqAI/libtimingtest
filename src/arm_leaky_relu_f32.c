#include <arm_math.h>
#include <arm_mve.h>

void
arm_leaky_relu_s8(int8_t *data, int32_t size, int8_t alpha)
{
    // leakyReLU = x if x > 0 else alpha * x
}

void
arm_leaky_relu_f32(float32_t *data, int32_t size, float32_t alpha)
{
    // leakyReLU = x if x > 0 else alpha * x
    uint32_t blkCnt = (size + 3) / 4;
    float32x4_t data_v, result_v;
    int32x4_t result_i32_v;
    mve_pred16_t mask;

    while (blkCnt > 0)
    {
        // Load 4 values
        data_v = vld1q_f32(data);

        // Multiply by alpha and store in result
        result_v = vmulq_f32(data_v, vdupq_n_f32(alpha));

        // Compare where data is less than 0
        mask = vcmpltq_f32(data_v, vdupq_n_f32(0.0f));

        // Reinterpret result as int32_t
        result_i32_v = vreinterpretq_s32_f32(result_v);

        // Store result
        vstrbq_p_s32(data, result_i32_v, mask);

        data += 4;
        blkCnt--;
    }


    // Store using predicate


}
