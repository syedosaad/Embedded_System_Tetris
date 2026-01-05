/* Author: Manyan Wong */

#ifndef SCORE_TRACKER_H
#define SCORE_TRACKER_H

// Function declarations
void save_high_scores(int new_score); // saves the new score to the high scores array
void display_high_scores(void); // displays the high scores on the 7 segment display
void swap(int *a, int *b); // swaps the values of two integers
void sortDescending(int scores[], int players[], int length); // sorts the scores in descending order

#endif
