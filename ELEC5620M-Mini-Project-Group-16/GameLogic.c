/*  Author: Shikha Tripathi, Saaduddin Syed, Manyan Wong, Harini Nagarathinam
 */

#include "GameLogic.h"
#include "LT24Display.h"
#include "DE1SoC_LT24/DE1SoC_LT24.h"
#include "LT24Display.h"
#include <stdlib.h>
#include <stdint.h>
#include "Audio.h"

const int BLOCK_SHAPES[7][4][4][4] = {
    // I-block
    {
        {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0}},
        {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}}
    },
    // O-block
    {
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}
    },
    // T-block
    {
        {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    // S-block
    {
        {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0}},
        {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
        {{1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    // Z-block
    {
        {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
        {{0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0}}
    },
    // J-block
    {
        {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0}}
    },
    // L-block
    {
        {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0}},
        {{1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    }
};


void init_game(GameState *game, LT24Ctx_t *ctx) {
    // Initialize game grid
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            game->grid[y][x] = -1;
        }
    }
    game->score = 0;
    game->lines_cleared = 0;
    game->level = 1;
    game->gameOver = 0;
    game->paused = 0;  // Initialize paused flag

    // Clear screen
//    LT24_clearDisplay(ctx, 0x0000);

    // Spawn first block
    spawn_new_block(game);
}

void update_gravity(GameState *game, LT24Ctx_t *ctx) {
    int newY = game->currentBlock.y + 1;
    erase_active_block(game, ctx);
    if (!check_collision(game, game->currentBlock.x, newY, game->currentBlock.rotation)) {
        game->currentBlock.y = newY;
    } else {
        lock_block(game, ctx);

        // Use the new function instead of clear_full_lines
        check_and_clear_lines(game, ctx);

        spawn_new_block(game);

        if (is_game_over(game)) {
            game->gameOver = 1;
        }
    }

    // Redraw after moving
    draw_game(game, ctx);
    draw_block(&game->currentBlock, ctx, get_block_color(game->currentBlock.type));
}


// Spawns a new random block at the top center
void spawn_new_block(GameState *game) {
    ActiveBlock *block = &game->currentBlock;

    // random block type and default rotation
    block->type = rand() % NUM_BLOCK_TYPES;
    block->rotation = 0;

    // spawn position: top-middle
    block->x = (GRID_WIDTH / 2) - 2;
    block->y = 0;

    // check if the new block collides with existing blocks for game over condition
    const int (*shape)[4] = get_block_shape(block->type, block->rotation);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j]) {
                int x = block->x + j;
                int y = block->y + i;
                if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
                    if (game->grid[y][x] != -1) {
                        game->gameOver = 1;
                        return;
                    }
                }
            }
        }
    }
}

void lock_block(GameState *game, LT24Ctx_t *ctx) {
    ActiveBlock *block = &game->currentBlock;
    const int (*shape)[4] = get_block_shape(game->currentBlock.type, game->currentBlock.rotation);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j]) {
                int x = game->currentBlock.x + j;
                int y = game->currentBlock.y + i;
                if (y >= 0 && y < GRID_HEIGHT && x >= 0 && x < GRID_WIDTH) {
                    game->grid[y][x] = game->currentBlock.type;
                }
            }
        }
    }

    // We don't need to call check_and_clear_lines here since
    // update_gravity already calls it after calling lock_block
}

// Clears all full lines and moves rows down
void clear_full_lines(GameState *game) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        int full = 1;
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (game->grid[y][x] == -1) {
                full = 0;
                break;
            }
        }
        if (full) {
            for (int row = y; row > 0; row--) {
                for (int col = 0; col < GRID_WIDTH; col++) {
                    game->grid[row][col] = game->grid[row - 1][col];
                }
            }
            for (int col = 0; col < GRID_WIDTH; col++) {
                game->grid[0][col] = -1;
            }
            game->score += 100;

        }
    }
}

// Checks if the block at a given position collides with existing blocks or borders
int check_collision(GameState *game, int x, int y, int rotation) {
    const int (*shape)[4] = get_block_shape(game->currentBlock.type, rotation);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j]) {
                int nx = x + j;
                int ny = y + i;

                if (nx < 0 || nx >= GRID_WIDTH || ny >= GRID_HEIGHT) {
                    return 1;
                }

                if (ny >= 0 && game->grid[ny][nx] != -1) {
                    return 1;
                }
            }
        }
    }
    return 0;
}


const int (*get_block_shape(int type, int rotation))[4] {
    return BLOCK_SHAPES[type][rotation];
}

// ----------------------------make this work----------------------------
int check_and_clear_lines(GameState *game, LT24Ctx_t *ctx) {
    int lines_cleared = 0;

    // Check each row from bottom to top
    for (int row = GRID_HEIGHT - 1; row >= 0; row--) {
        // Check if this row is full
        int is_full = 1;
        for (int col = 0; col < GRID_WIDTH; col++) {
            if (game->grid[row][col] == -1) {
                is_full = 0;
                break;
            }
        }

        // If row is full, clear it and shift rows above down
        if (is_full) {
            lines_cleared++;

            // Flash the line being cleared
            for (int flash = 0; flash < 3; flash++) {
                // Flash white
                for (int col = 0; col < GRID_WIDTH; col++) {
                    draw_block_at(ctx, row, col, 0xFFFF); // White
                }
                delay_ms(50);

                // Flash black
                for (int col = 0; col < GRID_WIDTH; col++) {
                    draw_block_at(ctx, row, col, 0x0000); // Black
                }
                delay_ms(50);
            }

            // Move all rows above this one down
            for (int move_row = row; move_row > 0; move_row--) {
                for (int col = 0; col < GRID_WIDTH; col++) {
                    game->grid[move_row][col] = game->grid[move_row - 1][col];
                }
            }

            // Clear the top row
            for (int col = 0; col < GRID_WIDTH; col++) {
                game->grid[0][col] = -1;
            }

            // Redraw the entire grid after shifting
            draw_game(game, ctx);

            // Check this row again since we've moved everything down
            row++;
        }
    }

    // Update score based on lines cleared
    if (lines_cleared > 0) {
        // Classic Tetris scoring: 40, 100, 300, 1200 points for 1, 2, 3, 4 lines
        switch (lines_cleared) {
            case 1:
                game->score += 40 * game->level;
                break;
            case 2:
                game->score += 100 * game->level;
                break;
            case 3:
                game->score += 300 * game->level;
                break;
            case 4:
                game->score += 1200 * game->level; // Tetris!
                break;
        }

        // Update lines cleared count
        game->lines_cleared += lines_cleared;

        // Level up every 10 lines
        game->level = 1 + (game->lines_cleared / 10);
    }

    return lines_cleared;
}
/*void draw_block(const ActiveBlock *block, LT24Ctx_t *ctx, uint16_t color) {
    const int (*shape)[4] = get_block_shape(block->type, block->rotation);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (shape[i][j]) {
                int px = (block->x + j);
                int py = (block->y + i);
                if (i >= 0 && i < GRID_HEIGHT && j >= 0 && j < GRID_WIDTH) {
                                    draw_block_at(ctx, i, j, color);
                    }
                }
            }
        }
    }*/

//Changed this need to be looked into
int is_game_over(GameState *game) {
    // Only check if the new block can't be placed at spawn position
    const int (*shape)[4] = get_block_shape(game->currentBlock.type, game->currentBlock.rotation);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j]) {
                int x = game->currentBlock.x + j;
                int y = game->currentBlock.y + i;
                // Only check collisions with existing blocks (not bounds)
                if (y >= 0 && y < GRID_HEIGHT && x >= 0 && x < GRID_WIDTH) {
                    if (game->grid[y][x] >= 0) {  // There's already a block here
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}
