/*  Author: Shikha Tripathi, Saaduddin Syed, Manyan Wong,
 */
#include "LT24Display.h"
#include "GameLogic.h"
#include <stdio.h>  //gonna remove this later
#include "BasicFont/BasicFont.h"
#include "DE1SoC_LT24/DE1SoC_LT24.h"


#define BLOCK_WIDTH  15
#define BLOCK_HEIGHT 15

#define NUM_BLOCK_TYPES 7

uint16_t get_block_color(BlockType type) {
    switch(type) {
        case I: return 0x07FF; // Cyan
        case J: return 0x001F; // Blue
        case L: return 0xFBE0; // Orange
        case O: return 0xFFE0; // Yellow
        case S: return 0x07E0; // Green
        case T: return 0xF81F; // Purple
        case Z: return 0xF800; // Red
        default: return 0xFFFF; // White for unknown
    }
}

void draw_block_at(LT24Ctx_t* ctx, int row, int col, uint16_t color) {
    // Calculate pixel coordinates from grid position
    int pixel_x = col * BLOCK_WIDTH;  // Add GRID_OFFSET_X if you have one
    int pixel_y = row * BLOCK_HEIGHT; // Add GRID_OFFSET_Y if you have one

    // Safety check for bounds
    if (row < 0 || row >= GRID_HEIGHT || col < 0 || col >= GRID_WIDTH) {
        printf("Error: draw_block_at out of bounds: row=%d col=%d\n", row, col);
        return;
    }

    // Draw a block at the specified grid position
    for (int dy = 0; dy < BLOCK_HEIGHT; dy++) {
        for (int dx = 0; dx < BLOCK_WIDTH; dx++) {
            LT24_drawPixel(ctx, color, pixel_x + dx, pixel_y + dy);
        }
    }
}

void draw_game(GameState* game,  LT24Ctx_t* ctx) {
    // Optional: Clear screen
//     LT24_clearDisplay(ctx,0x0000);

    for (int row = 0; row < GRID_HEIGHT; row++) {
        for (int col = 0; col < GRID_WIDTH; col++) {
            if (game->grid[row][col] >= 0 && game->grid[row][col] < NUM_BLOCK_TYPES) {
                uint16_t color = get_block_color((BlockType)game->grid[row][col]);
                draw_block_at(ctx, row, col, color);
            }
        }
    }

    ActiveBlock *block = &game->currentBlock;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            const int (*shape)[4] = get_block_shape(block->type, block->rotation);
            if (shape[r][c]) {
                int row = block->y + r;
                int col = block->x + c;
                if (row >= 0 && row < GRID_HEIGHT && col >= 0 && col < GRID_WIDTH) {
                    uint16_t color = get_block_color(block->type);
                    draw_block_at(ctx, row, col, color);
                }
            }
        }
    }
}

void erase_active_block(GameState* game, LT24Ctx_t* ctx) {
    ActiveBlock *block = &game->currentBlock;
    const int (*shape)[4] = get_block_shape(block->type, block->rotation);

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (shape[r][c]) {
                int row = block->y + r;
                int col = block->x + c;
                if (row >= 0 && row < GRID_HEIGHT && col >= 0 && col < GRID_WIDTH) {
                    draw_block_at(ctx, row, col, 0x0000); // Black
                }
            }
        }
    }
}

void draw_block(const ActiveBlock *block, LT24Ctx_t *ctx, uint16_t color) {
    // Safety check
    if (block == NULL || ctx == NULL) {
        return;
    }

    const int (*shape)[4] = get_block_shape(block->type, block->rotation);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j]) {
                int row = block->y + i;
                int col = block->x + j;
                if (row >= 0 && row < GRID_HEIGHT && col >= 0 && col < GRID_WIDTH) {
                    draw_block_at(ctx, row, col, color);
                }
            }
        }
    }
}

// function to display a single character on screen
void LT24_drawChar(LT24Ctx_t* ctx, int x, int y, char c, uint16_t fg, uint16_t bg) {
    if (c < ' ' || c > '~') return;  // skip out-of-range characters
    
    for (int col = 0; col < 5; col++) {  // loop through the 5 columns of the character
        unsigned char bits = BF_fontMap[c - ' '][col];  // get the bitmap column from font array
        for (int row = 0; row < 8; row++) {  // loop through each of the 8 rows
            // bit order is from bit 0 (top) to bit 7 (bottom)
            uint16_t color = (bits & (1 << row)) ? fg : bg;  // if bit is set, use foreground color, else background
            LT24_drawPixel(ctx, color, x + col, y + row);  // draw the pixel at the right position
        }
    }
}


// function to draw game over screen when player loses
void draw_game_over(LT24Ctx_t* ctx) {
    // adding a grey layer overlay over the entire game area
    for (int y = 0; y < GRID_HEIGHT * BLOCK_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH * BLOCK_WIDTH; x++) {
            //checkered pattern for semi-transparent effect
            if ((x + y) % 2 == 0) {
                LT24_drawPixel(ctx, 0x0000, x, y);  // black pixels for overlay
            }
        }
    }
    
    // clear portion of the screen for message box
    int rect_x = 20;  // left position of message box
    int rect_y = 80;  // top position of message box
    int rect_width = GRID_WIDTH * BLOCK_WIDTH - 40;  // width that fits the screen
    int rect_height = 50;  // height of the message box
    
    // draw red background box for the message
    for (int y = rect_y; y < rect_y + rect_height; y++) {
        for (int x = rect_x; x < rect_x + rect_width; x++) {
            LT24_drawPixel(ctx, 0xF800, x, y);  // red box
        }
    }
    
    // draw "game over" text
    const char* msg = "GAME OVER";  // text to display
    int len = 9;  // length of message (9 chars)
    int char_width = 6;  // each char 5 pixels plus 1 pixel spacing
    int text_width = len * char_width;
    int start_x = (GRID_WIDTH * BLOCK_WIDTH - text_width) / 2;
    int start_y = rect_y + 10;
    
    // draw each character of game over msg
    for (int i = 0; i < len; i++) {
        LT24_drawChar(ctx, start_x + i * char_width, start_y, msg[i], 0xFFFF, 0xF800);  // white text on red
    }
    
    // draw a white border around the rectangle for better visibility
    for (int x = rect_x; x < rect_x + rect_width; x++) {
        LT24_drawPixel(ctx, 0xFFFF, x, rect_y);  // top border line
        LT24_drawPixel(ctx, 0xFFFF, x, rect_y + rect_height - 1);  // bottom border line
    }
    for (int y = rect_y; y < rect_y + rect_height; y++) {
        LT24_drawPixel(ctx, 0xFFFF, rect_x, y);  // left border line
        LT24_drawPixel(ctx, 0xFFFF, rect_x + rect_width - 1, y);  // right border line
    }
}


// function to draw a GO message when game starts
void draw_game_start(LT24Ctx_t* ctx) {
    // clear the screen with black background
    LT24_clearDisplay(ctx, 0x0000);
    
    // create a green box for GO msg
    int rect_x = 45;  // left position of message box
    int rect_y = 80;  // top position of message box
    int rect_width = 60;  // width of the message box
    int rect_height = 40;  // height of the message box
    
    // draw green background rectangle for the message
    for (int y = rect_y; y < rect_y + rect_height; y++) {
        for (int x = rect_x; x < rect_x + rect_width; x++) {
            LT24_drawPixel(ctx, 0x07E0, x, y);  // green background (0x07E0 is green)
        }
    }
    
    // draw GO text for when game starts
    const char* msg = "GO";  				// text to display
    int len = 2;  							// length of text (2 characters)
    int char_width = 8;  					// each character is 5 pixels plus 3 pixel spacing
    int text_width = len * char_width;
    int start_x = rect_x + (rect_width - text_width) / 2;  //  text position horizontally
    int start_y = rect_y + (rect_height - 8) / 2;  // text position vertically
    
    // draw each character of GO msg
    for (int i = 0; i < len; i++) {
        LT24_drawChar(ctx, start_x + i * char_width, start_y, msg[i], 0xFFFF, 0x07E0);  // white text on green
    }
    
    // draw a white border around the rectangle
    for (int x = rect_x; x < rect_x + rect_width; x++) {
        LT24_drawPixel(ctx, 0xFFFF, x, rect_y);  // top border line
        LT24_drawPixel(ctx, 0xFFFF, x, rect_y + rect_height - 1);  // bottom border line
    }
    for (int y = rect_y; y < rect_y + rect_height; y++) {
        LT24_drawPixel(ctx, 0xFFFF, rect_x, y);  // left border line
        LT24_drawPixel(ctx, 0xFFFF, rect_x + rect_width - 1, y);  // right border line
    }
}

// Function to draw the pause screen overlay
void draw_pause(LT24Ctx_t* ctx) {
    // Adding a overlay over the entire game area
    for (int y = 0; y < GRID_HEIGHT * BLOCK_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH * BLOCK_WIDTH; x++) {
            //  create a semi-transparent effect
            if ((x + y) % 2 == 0) {
                LT24_drawPixel(ctx, 0x0000, x, y);  // black pixels for overlay
            }
        }
    }
    
    // Clear portion of the screen for message box
    int rect_x = 20; 
    int rect_y = 80; 
    int rect_width = GRID_WIDTH * BLOCK_WIDTH - 40;  // width that fits the screen
    int rect_height = 50;  // height of the message box
    
    // Draw blue background box for the message
    for (int y = rect_y; y < rect_y + rect_height; y++) {
        for (int x = rect_x; x < rect_x + rect_width; x++) {
            LT24_drawPixel(ctx, 0x001F, x, y);  // blue box
        }
    }
    
    // Draw "PAUSED" text
    const char* msg = "PAUSED";  // text to display
    int len = 6;  // length of message
    int char_width = 6; 
    int text_width = len * char_width;
    int start_x = (GRID_WIDTH * BLOCK_WIDTH - text_width) / 2;
    int start_y = rect_y + 20;  // Center the text vertically
    
    // Draw each character of the pause message
    for (int i = 0; i < len; i++) {
        LT24_drawChar(ctx, start_x + i * char_width, start_y, msg[i], 0xFFFF, 0x001F);  // white text on blue
    }
    
    // Draw a white border around the rectangle for better visibility
    for (int x = rect_x; x < rect_x + rect_width; x++) {
        LT24_drawPixel(ctx, 0xFFFF, x, rect_y);  // top border line
        LT24_drawPixel(ctx, 0xFFFF, x, rect_y + rect_height - 1);  // bottom border line
    }
    for (int y = rect_y; y < rect_y + rect_height; y++) {
        LT24_drawPixel(ctx, 0xFFFF, rect_x, y);  // left border line
        LT24_drawPixel(ctx, 0xFFFF, rect_x + rect_width - 1, y);  // right border line
    }
}


