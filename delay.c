#include "delay.h"

unsigned int delay_length_periods = 100;
double delay_feedback_level = 0.5;
double delay_mix_level = 1.0;

int16_t *delay_buf;
unsigned int delay_ptr = 0;

void delay_init() {
	delay_buf = malloc(sizeof(int16_t) * delay_length_periods * AUDIO_SOFT_BUF_SIZE_FRAMES);

	unsigned int i;
	for (i = 0; i < delay_length_periods * AUDIO_SOFT_BUF_SIZE_FRAMES; i++)
		delay_buf[i] = 0;
}

void delay_free() {
	free(delay_buf);
}

void delay_append(int16_t *buffer) {
	unsigned int i;
	for (i = 0; i < AUDIO_SOFT_BUF_SIZE_FRAMES; i++) {
		delay_buf[(delay_ptr * AUDIO_SOFT_BUF_SIZE_FRAMES) + i] *= delay_feedback_level;
		delay_buf[(delay_ptr * AUDIO_SOFT_BUF_SIZE_FRAMES) + i] += buffer[i];
	}

	delay_ptr++;
	if (delay_ptr == delay_length_periods)
		delay_ptr = 0;
}

void delay_apply(int16_t *buffer) {
	unsigned int i;
	for (i = 0; i < audio_period_size_frames * AUDIO_CHANNELS; i++)
		buffer[i] += delay_buf[(delay_ptr * AUDIO_SOFT_BUF_SIZE_FRAMES) + i] * delay_mix_level;
}
