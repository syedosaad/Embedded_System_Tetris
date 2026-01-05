/*
 * Audio.c
 *
 *  Created on: 14-May-2025
 *      Author: Harini Nagarathinam
 */

#include "Audio.h"
#include<stdio.h>
WM8731Ctx_t* audio;
HPSI2CCtx_t* hpsi2c;
HPSGPIOCtx_t* gpio;

unsigned int sampleRate = 48000;
double ampl = 8588608.0;
double phase = 0;
double inc = 0;
int remainingSamples=0;
int i; //index



void exitOnFail(HpsErr_t status)
{
    if (ERR_IS_ERROR(status))
        exit((int)status);
}

void d(int ms) {
    volatile int count;
    while (ms-- > 0) {
        count = 15000;
        while (count-- > 0);
    }
}

void init_audio()
{
	exitOnFail(HPS_GPIO_initialise(LSC_BASE_ARM_GPIO, ARM_GPIO_DIR, ARM_GPIO_I2C_GENERAL_MUX, 0, &gpio));
	exitOnFail(HPS_I2C_initialise(LSC_BASE_I2C_GENERAL, I2C_SPEED_STANDARD, &hpsi2c));
	exitOnFail(WM8731_initialise(LSC_BASE_AUDIOCODEC, &hpsi2c->i2c, &audio));
	WM8731_clearFIFO(audio, true, true);
	WM8731_getSampleRate(audio, &sampleRate);
}

void play_tone(double freq,double duration)
{
	inc = freq *M_2PI / sampleRate;
	phase = 0;
	remainingSamples=(int)(duration*sampleRate);
	while(remainingSamples>0)
	    update_audio();
}

void background_music() {
	play_tone(400,0.1);
	d(80);
}

void update_audio()
{
	if (remainingSamples <= 0) return;

    unsigned int space;
	while (remainingSamples >0)
	{
		WM8731_getFIFOSpace(audio, &space);
		if ( space >0 )
		{
			phase += inc;
			if (phase >= M_2PI) phase -= M_2PI;
			int sample = (int)(ampl*sin(phase));
			WM8731_writeSample(audio, sample, sample);
			remainingSamples--;
		}
		ResetWDT();
	}
}

void stop_music()
{
	play_tone(0,0.1);
	update_audio();
}
void scoretrack()
{
	play_tone(783.99,0.1);
	d(80);
	play_tone(880,0.1);
	d(80);
	play_tone(987.77,0.1);
	d(80);
}

void pausetrack()
{
	play_tone(523.25,0.15);
	d(80);
	play_tone(659.25,0.15);
	d(80);
}

void gameovertrack()
{
	play_tone(659.25,0.3);
	d(80);
	play_tone(523.25,0.3);
	d(80);
	play_tone(440,0.5);
	d(80);
}

void starttrack()
{
	play_tone(659.25,0.3);
	d(80);
	play_tone(493.88,0.3);
	d(80);
	play_tone(523.25,0.5);
	d(80);
	play_tone(587.33,0.3);
	d(80);
	play_tone(523.25,0.3);
	d(80);
	play_tone(493.88,0.3);
	d(80);
	play_tone(440,0.6);
	d(80);
}
