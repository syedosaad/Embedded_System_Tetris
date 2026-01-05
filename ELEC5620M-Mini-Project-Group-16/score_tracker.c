/*  Author: Manyan Wong */
#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include "score_tracker.h"
#include "seven_segment.h"
#include "GameLogic.h"
#include "Audio.h"

#define MAX_HIGH_SCORES 5
#define SCORE_FILENAME "high_scores.dat"

// Structure to hold high scores
typedef struct {
    int scores[MAX_HIGH_SCORES];
    int count;
} HighScores;

static HighScores highScores = {{0}, 0};

// Function to save score if it's a high score
void save_high_scores(int new_score) {
    // Initialize if first time
    if (highScores.count == 0) {
        // Try to load existing scores
        FILE *file = fopen(SCORE_FILENAME, "rb");
        if (file) {
            fread(&highScores, sizeof(HighScores), 1, file);
            fclose(file);
        }
    }
    
    // Check if new score is a high score
    int position = -1;
    for (int i = 0; i < highScores.count; i++) {
        if (new_score > highScores.scores[i]) {
            position = i;
            break;
        }
    }
    
    if (position == -1 && highScores.count < MAX_HIGH_SCORES) {
        // Not higher than existing scores but we have room
        position = highScores.count;
    }
    
    if (position != -1) {
        // Shift scores down to make room
        for (int i = highScores.count - 1; i > position; i--) {
            highScores.scores[i] = highScores.scores[i-1];
        }
        
        // Insert new score
        highScores.scores[position] = new_score;
        
        // Update count if needed
        if (highScores.count < MAX_HIGH_SCORES) {
            highScores.count++;
        }
        
        // Save scores
        FILE *file = fopen(SCORE_FILENAME, "wb");
        if (file) {
            fwrite(&highScores, sizeof(HighScores), 1, file);
            fclose(file);
        }
    }
}

// Function to display high scores
void display_high_scores(void) {
    printf("High Scores:\n");
    for (int i = 0; i < highScores.count; i++) {
        printf("%d. %d\n", i+1, highScores.scores[i]);
        
        // Display high score on seven segment display for a few seconds
        update_score_display(highScores.scores[i]);
        delay(5000000);  // Display each score for some time
    }
}


// Function to swap two integers
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Function to sort scores and players in descending order
void sortDescending(int scores[], int players[], int length) {
    for (int i = 0; i < length - 1; i++) {
        for (int j = i + 1; j < length; j++) {
            if (scores[i] < scores[j]) {
                swap(&scores[i], &scores[j]);
                swap(&players[i], &players[j]);
            }
        }
    }
}
