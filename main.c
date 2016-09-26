#include "audio.h"
#include "delay.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void print_usage()
{
	printf("Usage: dsp [OPTION]\n");
	printf("  -h		display this help and exit\n");
	printf("  -f		feedback level in (0..1) range\n");
	printf("  -l		delay length in milliseconds (1..1000)\n");
	printf("  -m		mix level (0..1)\n");
}

int main(int argc, char *argv[]) {
	int opt;
	int iarg;
	double farg;
	int16_t *audio_buf;

    audio_buf = malloc(AUDIO_SOFT_BUF_SIZE_BYTES);

    delay_length_periods = ceil((double)500 / ((double)audio_period_size_frames * 1000.0 / (double)audio_samp_rate));

    while ((opt = getopt(argc, argv, "hf:l:m:")) != -1) {
    	switch (opt) {
    	case 'h':
    		print_usage();
    		return EXIT_SUCCESS;
    		break;
    	case 'f':
    		farg = atof(optarg);

    		if ((farg >= 0.0) && (farg <= 1.0)) {
    			delay_feedback_level = farg;
    			printf("Feedback level set to %f\n", farg);
    		} else {
    			printf("Feedback level must be in (0..1) range; using default value %f\n", delay_feedback_level);
    		}

    		break;
    	case 'l':
    		iarg = atoi(optarg);

    		if ((iarg > 0) && (iarg <= 1000)) {
    			delay_length_periods = ceil((double)iarg / ((double)audio_period_size_frames * 1000.0 / (double)audio_samp_rate));
    			printf("Actual delay length will be %f ms\n", 1000.0 * (double)delay_length_periods * (double)audio_period_size_frames / (double)audio_samp_rate);
    		} else {
    			printf("Delay length must be in (1..1000) ms range; using default value 500\n");
    		}

    		break;
    	case 'm':
    		farg = atof(optarg);

    		if ((farg >= 0.0) && (farg <= 1.0)) {
    			delay_mix_level = farg;
    			printf("Mix level set to %f\n", farg);
    		} else {
    			printf("Mix level must be in (0..1) range; using default value %f\n", delay_mix_level);
    		}

    		break;
    	};
    }

    delay_init();

	if (!audio_init()) {
		printf("Audio initialization error, exiting\n");
		return EXIT_FAILURE;
	}

    while (1) {
    	if (!audio_read((char*)audio_buf, audio_period_size_frames)) {
    		audio_reinit();
    		continue;
    	}

    	delay_append(audio_buf);
    	delay_apply(audio_buf);

		if (!audio_write((char*)audio_buf, audio_period_size_frames)) {
			audio_reinit();
			continue;
		}
    }

    audio_free();
	delay_free();
	free(audio_buf);

	return EXIT_SUCCESS;
}
