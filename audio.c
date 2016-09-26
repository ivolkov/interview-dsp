#include "audio.h"
#include <sched.h>

/* set desired audio parameters here */
#define BUF_FRAMES	1024
#define SAMP_RATE	44100

/* device name */
const char *audio_p_device = "default";
//const char *audio_p_device = "hw:0,0";

/* sampling rate */
unsigned int audio_samp_rate = SAMP_RATE;

/* buffer size in frames */
unsigned int audio_buf_size_frames = BUF_FRAMES;

/* period between interrupts in frames */
unsigned int audio_period_size_frames = BUF_FRAMES / 2;

snd_pcm_t *p_handle;

#define CHECK_ERR(func) { if ((err = func) < 0) { printf("Initialization error: %s\n", snd_strerror(err)); return false; } }

bool audio_init()
{
    int err;

    snd_pcm_hw_params_t *p_hw_params;
    snd_pcm_sw_params_t *p_sw_params;

    /* set scheduler */
    struct sched_param sched_param;
    if (sched_getparam(0, &sched_param) < 0) {
        printf("Scheduler getparam failed...\n");
    } else {
        sched_param.sched_priority = sched_get_priority_max(SCHED_RR);

        if (!sched_setscheduler(0, SCHED_RR, &sched_param))
            printf("Scheduler set to Round Robin with priority %i...\n", sched_param.sched_priority);
        else
            printf("Scheduler set to Round Robin with priority %i FAILED\n", sched_param.sched_priority);
    }

    snd_pcm_hw_params_alloca(&p_hw_params);
    snd_pcm_sw_params_alloca(&p_sw_params);

    /* open device */
    CHECK_ERR( snd_pcm_open(&p_handle, audio_p_device, SND_PCM_STREAM_PLAYBACK, 0) )

    /* read current parameters */
    CHECK_ERR( snd_pcm_hw_params_any(p_handle, p_hw_params) )

    /* set access mode */
    CHECK_ERR( snd_pcm_hw_params_set_access(p_handle, p_hw_params, AUDIO_ACCMODE) )

    /* set data format */
    CHECK_ERR( snd_pcm_hw_params_set_format(p_handle, p_hw_params, AUDIO_FORMAT) )

    /* set number of channels */
    CHECK_ERR( snd_pcm_hw_params_set_channels(p_handle, p_hw_params, AUDIO_CHANNELS) )

    /* set sampling rate */
    unsigned int p_rate = audio_samp_rate;
    CHECK_ERR( snd_pcm_hw_params_set_rate_near(p_handle, p_hw_params, &p_rate, 0) )

    if (p_rate != audio_samp_rate)
    	printf("Warning: actual playback sampling rate will be %u\n", p_rate);

    audio_samp_rate = p_rate;

    /* set buffers size */
    snd_pcm_uframes_t p_buf_size = audio_buf_size_frames;

    CHECK_ERR( snd_pcm_hw_params_set_buffer_size_near(p_handle, p_hw_params, &p_buf_size) )

    if (p_buf_size != audio_buf_size_frames)
    	printf("Warning: actual playback buffer size will be %u frames\n", (unsigned int)p_buf_size);

    audio_buf_size_frames = p_buf_size;

    /* set interrupt period */
    snd_pcm_uframes_t p_period = audio_period_size_frames;

    CHECK_ERR( snd_pcm_hw_params_set_period_size_near(p_handle, p_hw_params, &p_period, 0) )

    if (p_period != audio_period_size_frames)
        printf("Warning: actual playback period size will be %u frames\n", (unsigned int)p_period);

    audio_period_size_frames = p_period;

    /* write parameters */
    CHECK_ERR( snd_pcm_hw_params(p_handle, p_hw_params) )

    return true;
}

void audio_reinit()
{
	char buf[AUDIO_SOFT_BUF_SIZE_BYTES * AUDIO_CHANNELS];
	unsigned int i;

	for (i = 0; i < AUDIO_SOFT_BUF_SIZE_BYTES * AUDIO_CHANNELS; i++)
		buf[i] = 0;

    printf("Audio reinitialization\n");

    snd_pcm_prepare(p_handle);
    snd_pcm_writei(p_handle, buf, audio_buf_size_frames);
}

bool audio_read(char *buffer, snd_pcm_sframes_t frames_num)
{
	ssize_t tmp;

	tmp = read(0, buffer, frames_num * AUDIO_FRAME_SIZE_BYTES * AUDIO_CHANNELS);

	return (tmp > 0);
}

bool audio_write(char *buffer, snd_pcm_sframes_t frames_num)
{
    if ((frames_num = snd_pcm_writei(p_handle, buffer, frames_num)) < 0) {
        printf("Audio write error: %s\n", snd_strerror(frames_num));
        return false;
    }

    return true;
}

void audio_free()
{
    snd_pcm_drain(p_handle);
    snd_pcm_hw_free(p_handle);
    snd_pcm_close(p_handle);
}
