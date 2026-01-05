/*  Author: Manyan Wong, Harini Nagarathinam
 */

#include "seven_segment.h"
#include "Audio.h"

#define HEX_DISPLAY_3_0   ((volatile uint32_t *) 0xFF200020)
#define HEX_DISPLAY_5_4   ((volatile uint32_t *) 0xFF200030)

uint8_t hex_digit[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

// Delay loop for debounce
void delay(volatile int count) {
    while (count-- > 0);
}

void display_number(int num) {
    uint32_t value_3_0 = 0;  // hold pattern for HEX3-0  (4 digits on the right of the seven segment)
    uint32_t value_5_4 = 0; //hold pattern for HEX5-4 (2 left digits of the seven segment)

    // Lower 4 digits (HEX3-0)
    for (int i = 0; i < 4; i++) {
        int digit = num % 10;
        num /= 10;
        value_3_0 |= (hex_digit[digit] << (i * 8));
    }
    // Upper 2 digits (HEX5-4)
    for (int i = 0; i < 2; i++) {
        int digit = num % 10;
        num /= 10;
        value_5_4 |= (hex_digit[digit] << (i * 8));
    }

    *HEX_DISPLAY_3_0 = value_3_0;
    *HEX_DISPLAY_5_4 = value_5_4;
}

void update_score_display(int score) {
    display_number(score);
    background_music();
    if (score != 0)
    {
    	scoretrack();
    	stop_music();
    	background_music();
    }
}
