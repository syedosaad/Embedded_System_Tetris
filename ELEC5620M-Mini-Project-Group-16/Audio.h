/*
 * Audio.h
 *
 *  Created on: 14-May-2025
 *      Author: Harini Nagarathinam
 */

#ifndef AUDIO_H_
#define AUDIO_H_

#include "Util/macros.h"
#include "DE1SoC_Addresses/DE1SoC_Addresses.h"
#include "DE1SoC_WM8731/DE1SoC_WM8731.h"
#include "HPS_I2C/HPS_I2C.h"
#include "Util/watchdog.h"
#include "HPS_GPIO/HPS_GPIO.h"
#include "FPGA_PIO/FPGA_PIO.h"
#include <math.h>
#include <stdlib.h>
#include<stdint.h>

void init_audio();
void background_music();
void update_audio();
void scoretrack();
void pausetrack();
void gameovertrack();
void starttrack();
void stop_music();

#endif /* AUDIO_H_ */
