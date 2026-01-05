/*  Author: , Manyan Wong,
 */
#ifndef SEVEN_SEGMENT_H
#define SEVEN_SEGMENT_H

#include <stdint.h>

// Function declarations
void update_score_display(int score); // responsible for taking a game score and displaying it on the 7 segment display
void display_number(int num); //displays a whole number on the 7 segment display
void delay(volatile int count); // creates a delay

#endif
