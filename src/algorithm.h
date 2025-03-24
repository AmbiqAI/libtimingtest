#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <stdbool.h>
#include <arm_math.h>
#include <arm_math_f16.h>


/*-------------------- Algorithm public constants ----------------------------*/
#define MAX_INPUT_CHANNELS 20
#define BUFFER_LEN 250

/*-------------------- Algorithm public structs ------------------------------*/
/**
 * @brief Circular buffer
 *
 * @param values   values of the buffer
 * @param position position of the buffer
 */
struct buffer {
    float16_t values[BUFFER_LEN];
    int position;
};

/**
 * @brief Algorithm input
 *
 * @param channels        input channels
 * @param num_channels    number of input channels
 * @param sample_interval sample interval
 */
struct algorithm_input {
    float16_t channels[MAX_INPUT_CHANNELS];
    float16_t num_channels;
    float16_t sample_interval;
};

/**
 * @brief Algorithm instance
 *
 * @param buffer input buffers
 * @param time   timer of the algorithm
 */
struct algorithm {
    struct buffer buffer[MAX_INPUT_CHANNELS];
    float16_t time;
};

/*-------------------- Algorithm function declarations------------------------*/
/**
 * @brief Initialize algorithm instance
 *
 * @param instance pointer to the algorithm instance
 */
void algorithm_init(struct algorithm *instance);

/**
 * @brief Algorithm main function
 *
 * @param instance   pointer to the algorithm instance
 * @param input      pointer to the algorithm input
 * @param output     pointer to the algorithm output
 * @param tempBuffer pointer to the temporary buffer (len 3 * 1024 + 512)
 * @return bool: true if the output is available
 */
bool algorithm(struct algorithm *instance,
               struct algorithm_input *input,
               float16_t *tempBuffer,
               float16_t *output);

/*-------------------- Input generator public structs ------------------------*/
/**
 * @brief Structure for input generator
 *
 * @param frequency   frequency of the input signal
 * @param amplitude   amplitude of the input signal
 * @param phase       phase of the input signal
 * @param sample_rate sample rate of the input signal
 * @param time        time of the input signal
 */
struct input_generator {
    float frequency;
    float amplitude;
    float phase;
    float sample_rate;
    float time;
};

/*-------------------- Input generator function declarations -----------------*/
/**
 * @brief Initialize the input generator
 *
 * @param self        pointer to the input generator
 * @param frequency   frequency of the input signal
 * @param amplitude   amplitude of the input signal
 * @param phase       phase of the input signal
 * @param sample_rate sample rate of the input signal
 */
void input_generator_init(struct input_generator *self,
                          float frequency,
                          float amplitude,
                          float phase,
                          float sample_rate);

/**
 * @brief Generate a sine wave
 *
 * @param self pointer to the input generator
 * @return float16_t: the value of the sine wave
 */
float16_t generate_sine(struct input_generator *self);

/**
 * @brief Generate white noise
 *
 * @param amplitude amplitude of the white noise
 * @return float16_t: the value of the white noise
 */
float16_t generate_white_noise(float16_t amplitude);

#endif // ALGORITHM_H
