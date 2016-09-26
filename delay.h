#ifndef DELAY_H_
#define DELAY_H_

#include "audio.h"
#include <stdint.h>

extern unsigned int delay_length_periods;
extern double delay_feedback_level;
extern double delay_mix_level;

void delay_init();
void delay_free();
void delay_append(int16_t *buffer);
void delay_apply(int16_t *buffer);

#endif /* DELAY_H_ */
