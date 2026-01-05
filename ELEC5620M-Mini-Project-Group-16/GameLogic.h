/*  Author: Shikha Tripathi, Saaduddin Syed, Manyan Wong, Harini Nagarathinam
 */
#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdint.h>
#include "DE1SoC_LT24/DE1SoC_LT24.h"

#define GRID_WIDTH 10
#define GRID_HEIGHT 20

typedef enum { I, O, T, S, Z, J, L, NUM_BLOCK_TYPES } BlockType;

typedef struct {
    int x; // column
    int y; // row
    BlockType type;
    int rotation;
} ActiveBlock;

typedef struct {
    int grid[GRID_HEIGHT][GRID_WIDTH];
    ActiveBlock currentBlock;
    int gameOver;
    int paused;  // New paused state flag
    int score;
    int level;
    int lines_cleared;
} GameState;

// Function declarations
void init_game(GameState *game, LT24Ctx_t *ctx);
void update_gravity(GameState *game, LT24Ctx_t *ctx);
void spawn_new_block(GameState *game);
void lock_block(GameState *game, LT24Ctx_t *ctx);
void clear_full_lines(GameState *game);
int check_collision(GameState *game, int x, int y, int rotation);
const int (*get_block_shape(int type, int rotation))[4];
void draw_block(const ActiveBlock *block, LT24Ctx_t *ctx, uint16_t color);
int is_game_over(GameState *game);
// Clear line declaration
void delay_ms(int ms);  // For the animation in line clearing
int check_and_clear_lines(GameState *game, LT24Ctx_t *ctx);  // Line clearing function

#endif
