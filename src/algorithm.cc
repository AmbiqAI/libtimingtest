#include "algorithm.h"
#include <arm_math.h>
#include <arm_math_f16.h>
#include <math.h>
#include <stdlib.h>

/*-------------------- Circular buffer function declarations -----------------*/
void buffer_init(struct buffer *instance);
void buffer_input(struct buffer *instance, float16_t value);
void buffer_copy(struct buffer *instance, float16_t *output);

/*-------------------- Algorithm private constants ---------------------------*/
#define CALCULATION_INTERVAL 1

static float16_t hann_window[1024];
static arm_rfft_fast_instance_f16 fft;


/*-------------------- Algorithm function definitions ------------------------*/
void algorithm_init(struct algorithm *instance) {
    instance->time = 0;
    for (int i = 0; i < MAX_INPUT_CHANNELS; i++) {
        buffer_init(&instance->buffer[i]);
    }
    // Initialize Hann window
    for (int i = 0; i < 1024; i++) {
        hann_window[i] = 0.5f * (1 - cosf(2 * PI * i / 1024));
    }
    arm_rfft_fast_init_1024_f16(&fft);
}

bool algorithm(struct algorithm *instance,
               struct algorithm_input *input,
               float16_t *tempBuffer,
               float16_t *output) {
    for (int channel = 0; channel < input->num_channels; channel++) {
        buffer_input(&instance->buffer[channel], input->channels[channel]);
    }

    instance->time += input->sample_interval;
    if (instance->time < CALCULATION_INTERVAL) {
        return false;
    }
    instance->time -= CALCULATION_INTERVAL;

    float16_t *sBuffer = &tempBuffer[0]; // 1024
    arm_fill_f16(0, sBuffer, 1024);

    for (int channel = 0; channel < input->num_channels; channel++) {
        float16_t *tBuffer = &tempBuffer[1024]; // 1024
        float16_t *fBuffer = &tempBuffer[2 * 1024]; // 1024

        buffer_copy(&instance->buffer[channel], tBuffer);

        arm_mult_f16(tBuffer, hann_window, tBuffer, BUFFER_LEN);
        arm_fill_f16(0, tBuffer + BUFFER_LEN, 1024 - BUFFER_LEN);
        arm_rfft_fast_f16(&fft, tBuffer, fBuffer, 0);
        arm_add_f16(fBuffer, sBuffer, sBuffer, 1024);
    }

    float16_t *mBuffer = &tempBuffer[3 * 1024]; // 512

    arm_cmplx_mag_squared_f16(sBuffer, mBuffer, 512);

    float16_t maxValue = 0;
    uint32_t maxIndex = 0;
    arm_max_f16(mBuffer, 512, &maxValue, &maxIndex);
    output[0] = maxIndex / (1024 * input->sample_interval);

    return true;
}

/*-------------------- Circular buffer function definitions ------------------*/
void buffer_init(struct buffer *instance) {
    instance->position = 0;
    for (int i = 0; i < BUFFER_LEN; i++) {
        instance->values[i] = 0;
    }
}

void buffer_input(struct buffer *instance, float16_t value) {
    instance->values[instance->position] = value;
    instance->position += 1;
    if (instance->position >= BUFFER_LEN) {
        instance->position = 0;
    }
}

void buffer_copy(struct buffer *instance, float16_t *output) {
    arm_copy_f16(&instance->values[instance->position], output, BUFFER_LEN - instance->position);
    arm_copy_f16(&instance->values[0], output + BUFFER_LEN - instance->position, instance->position);
}

/*-------------------- Input generator function definitions ------------------*/
void input_generator_init(struct input_generator *self,
                          float frequency,
                          float amplitude,
                          float phase,
                          float sample_rate) {
    self->frequency = frequency;
    self->amplitude = amplitude;
    self->phase = phase;
    self->sample_rate = sample_rate;
    self->time = 0;
}

float16_t generate_sine(struct input_generator *self) {
    float16_t result = self->amplitude *
        sinf(2 * PI * self->frequency * self->time + self->phase);

    self->time += 1 / self->sample_rate;
    if (self->time > 1 / self->frequency)
        self->time -= 1 / self->frequency;

    return result;
}

float16_t generate_white_noise(float16_t amplitude) {
    return amplitude * (2 * ((float16_t)rand() / RAND_MAX) - 1);
}
