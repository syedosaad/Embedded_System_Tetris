# ELEC5620M Mini-Project Repository - Group 16

# Tetris Game for DE1-SoC Board

-A Tetris game implementation for the Altera DE1-SoC FPGA development board. 
-The game displays on an LT24 LCD screen and uses the board's physical inputs for controls.

## Authors
Shikha Tripathi
Saaduddin Syed
Manyan Wong
Harini Nagarathinam

## Overview

The game follows standard Tetris rules:
- Blocks fall from the top of the screen
- Player can move, rotate, and drop blocks
- Completing full rows clears them and awards points
- Game ends when blocks stack to the top

## Controls

The game is controlled using the DE1-SoC's buttons and switches:
- KEY0: Move block right
- KEY1: Move block left
- KEY2: Rotate block clockwise
- KEY3: Hard drop (instantly drop block to bottom)
- SW0: Pause/Resume Game
## functionality

- 7 different shapes
- Score tracking on seven-segment display
- Game over screen with final scor- pause functionality
- Audio feedback
- Game Over screen

## Game Rules

-Blocks fall from the top of the screen
-Arrange blocks to create complete horizontal lines
-Completed lines are cleared and score points
-Game speed increases with higher scores
-Game ends when blocks stack to the top

## Project Structure
-main.c - Main game loop and initialization
-GameLogic.h/c - Core tetris game logic
-LT24Display.h/c - Display handling functions
-Audio.h/c - Sound effects
-seven_segment.h/c - 7-segment display functionality
-score_tracker.h/c - Score tracking and high score management

## Hardware addresses:
-Pushbuttons: 0xFF200050
-LEDs: 0xFF200000
-Switches: 0xFF200040
-7-segment display: 0xFF200020 & 0xFF200030
-Button inputs use edge detection for responsive controls
-Watchdog timer is reset regularly to prevent auto-restart
