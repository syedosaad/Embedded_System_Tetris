/*  Author: Shikha Tripathi, Saaduddin Syed, Manyan Wong, Harini Nagarathinam
 */
#include <stdio.h>
#include <stdlib.h>
#include "FPGA_PIO/FPGA_PIO.h"
#include "DE1SoC_Addresses/DE1SoC_Addresses.h"
#include "DE1SoC_LT24/DE1SoC_LT24.h"
#include "LT24Display.h"
#include "GameLogic.h"
#include <time.h>
#include "Audio.h"
#include "HPS_Watchdog/HPS_Watchdog.h"
#include "BasicFont/BasicFont.h"
#include "seven_segment.h"
#include "score_tracker.h"

#define PUSHBUTTONS_CONTROL 0xFF200050  //  Assign pushbuttons address to PUSHBUTTONS_CONTROL
volatile uint32_t *pushbuttons_ptr = (volatile uint32_t *)PUSHBUTTONS_CONTROL;  // Volatile unsigned 32 bit pointer for pushbuttons

#define LEDR_BASE 0xFF200000    //  Assign LEDR address to LEDR_BASE
volatile uint32_t *ledr_ptr = (volatile uint32_t *)LEDR_BASE;   // Volatile unsigned 32 bit pointer for LEDs

// Key positions - used for better readability when checking buttons
#define KEY1 0x1  // Move left
#define KEY0 0x2  // Move right
#define KEY2 0x4  // Rotate
#define KEY3 0x8  // Drop
#define SW0  0x1  // Pause

//  Store previous button state for edge detection
static uint32_t prev_buttons = 0;   //  Setting previous button state to zero
static int update_count = 0;    //  Initializing update_count as zero

// Define address for switches
#define SWITCHES_CONTROL 0xFF200040
volatile uint32_t *switches_ptr = (volatile uint32_t *)SWITCHES_CONTROL;

void poll_inputs(GameState *game, LT24Ctx_t *ctx) {
    uint32_t buttons = (*pushbuttons_ptr) & 0xF; // active low, mask to 4 bits
    uint32_t switches = *switches_ptr & 0x1;     // Read SW0 for pause

    //  Mapping pushbuttons to LEDR for debugging
    *ledr_ptr = buttons & 0xF; // Active low, mask to 4 bits

    uint32_t changed = ~buttons & prev_buttons; // Edge detection on button release

    // First check for pause toggle (SW0)
    static uint32_t prev_switches = 0;
    uint32_t switch_changed = switches ^ prev_switches;
    
    if (switch_changed & SW0) {
        // Toggle pause state
        game->paused = !game->paused;
        
        // If we just paused, draw the pause screen
        if (game->paused) {
            draw_pause(ctx);
            pausetrack();
        } else {
            // If we're unpausing, clear screen and redraw the game
            LT24_clearDisplay(ctx, 0x0000); // Clear the screen first
            draw_game(game, ctx);
            // Redraw active block
            draw_block(&game->currentBlock, ctx, get_block_color(game->currentBlock.type));
        }

    }
    
    prev_switches = switches;
    
    // Only process game inputs if not paused
    if (!game->paused) {
        if (changed & KEY1) { // Move left when KEY1 is released
            erase_active_block(game, ctx);  //  Erase the block from its current position
            if (!check_collision(game, game->currentBlock.x + 1, game->currentBlock.y, game->currentBlock.rotation)) {      // Check collision
                game->currentBlock.x++; // Move block left if no collision
            }
            draw_block(&game->currentBlock, ctx, get_block_color(game->currentBlock.type)); // Draw block at new position with its color
        }

        if (changed & KEY0) { // Move right when KEY0 is released
            erase_active_block(game, ctx);  //  Erase the block from its current position
            if (!check_collision(game, game->currentBlock.x - 1, game->currentBlock.y, game->currentBlock.rotation)) {      // Check collision
                game->currentBlock.x--; // Move block right if no collision
            }
            draw_block(&game->currentBlock, ctx, get_block_color(game->currentBlock.type)); // Draw block at new position with its color
        }

        if (changed & KEY2) { // Rotate when KEY2 is released
            erase_active_block(game, ctx);  //  Erase the block from its current position
            int new_rotation = (game->currentBlock.rotation + 1) % 4;   // Rotate block clockwise
            if (!check_collision(game, game->currentBlock.x, game->currentBlock.y, new_rotation)) { // Check collision
                game->currentBlock.rotation = new_rotation; // Rotate block if no collision
            }
            draw_block(&game->currentBlock, ctx, get_block_color(game->currentBlock.type)); // Draw block at new position with its color
        }

        if (changed & KEY3) { // Drop when KEY3 is released
            erase_active_block(game, ctx);  //  //  Erase the block from its current position
            while (!check_collision(game, game->currentBlock.x, game->currentBlock.y + 1, game->currentBlock.rotation)) {   // Check collision
                game->currentBlock.y++; // Drop block down if no collision
            }
            draw_block(&game->currentBlock, ctx, get_block_color(game->currentBlock.type)); // Draw block at new position with its color
        }
    }

    prev_buttons = buttons; // Reset button state
}

void delay_ms(int ms) { // Function to create a delay in milliseconds
    for (volatile int i = 0; i < 5000 * ms; i++); // Adjust the multiplier for the desired delay
}

int main(void) {
    HPS_ResetWatchdog();	//  To stop game from stopping in certain time
    printf("=== Starting Tetris on LT24 ===\n");
    init_audio();
    srand(time(NULL)); // seed random number generator (for random tetris blocks)

    FPGAPIOCtx_t* gpioCtx;     // pointer to the GPIO controller context for LT24 interface
    LT24Ctx_t* lt24Ctx;        // pointer to the LT24 display context

    // initialize FPGA GPIO pins used to bit-bang data to the LT24 screen
    if (FPGA_PIO_initialise(LSC_BASE_GPIO_JP1, LSC_CONFIG_GPIO, &gpioCtx) != ERR_SUCCESS) {
        printf("FPGA PIO init failed.\n");
        return 1;
    }

    // initialize LT24 display using GPIO
    if (LT24_initialise(&gpioCtx->gpio, NULL, &lt24Ctx) != ERR_SUCCESS) {
        printf("LT24 init failed.\n");
        return 1;
    }

    // display Go message at start of game
    draw_game_start(lt24Ctx);
    starttrack();
    delay_ms(500);  // display msg for 0.5 secs
    
    LT24_clearDisplay(lt24Ctx, 0x0000); 	// clear/black screen

    GameState game; 	// variable to hold game state
    init_game(&game, lt24Ctx); // set up initial game state, spawn first block and draw grid

    // Initialize score display on seven segment
    update_score_display(0);

    delay_ms(500); //Added start delay here
    // main loop continues until game over condition is triggered
    while (!game.gameOver) {
        // poll for button inputs to control the game
        poll_inputs(&game, lt24Ctx);

        // reset watchdog timer to keep game running
        HPS_ResetWatchdog();

        // Only update game logic if not paused
        if (!game.paused) {
            // only update gravity every few frames to control game speed
            if (++update_count >= 10) {  // adjust this value to make game faster/slower
                update_count = 0;
                update_gravity(&game, lt24Ctx);

                // Update score on seven segment display
                update_score_display(game.score);
            }
        }

        delay_ms(20);  // 20ms delay per frame
    }

    printf("GAME OVER!\n"); 		// print game over in console
    gameovertrack();
    draw_game(&game, lt24Ctx);  // draw final game state
    draw_game_over(lt24Ctx);		// display game over message on top of final game state
    
    // Save and display high scores on seven segment
    save_high_scores(game.score);
    display_high_scores();
    
    // game will stay on game over screen until board is reset
    while (1) {
        HPS_ResetWatchdog();   // reset watchdog to avoid game restarting automatically
        delay_ms(1000);
    }
    
    return 0;
}
