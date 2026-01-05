/*  Author: Shikha Tripathi, Saaduddin Syed, Manyan Wong,
 */
#ifndef LT24DISPLAY_H
#define LT24DISPLAY_H

#include "DE1SoC_LT24/DE1SoC_LT24.h"
#include "GameLogic.h"

// Constants
#define BLOCK_WIDTH  15
#define BLOCK_HEIGHT 15

// Function declarations ONLY (not implementations)
uint16_t get_block_color(BlockType type);
void draw_block_at(LT24Ctx_t* ctx, int row, int col, uint16_t color);
void draw_block(const ActiveBlock *block, LT24Ctx_t *ctx, uint16_t color);
void draw_game(GameState* game, LT24Ctx_t* ctx);
void erase_active_block(GameState* game, LT24Ctx_t* ctx);
void draw_game_over(LT24Ctx_t *ctx);
void LT24_drawChar(LT24Ctx_t* ctx, int x, int y, char c, uint16_t fg, uint16_t bg);
void draw_game_start(LT24Ctx_t *ctx);
void draw_pause(LT24Ctx_t *ctx);

#endif
