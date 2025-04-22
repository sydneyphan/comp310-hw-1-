#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "rprintf.h"  // Make sure this header is present along with rprintf.c

// The "terminal" where the text goes
#define NUM_COLS       80
#define NUM_ROWS       25
#define VIDEO_ADDRESS  0xB8000
#define DEFAULT_COLOR  0x07   // Light grey

// A pointer to video memory. 16 bits - lower 8 for ASCII, upper 8 for attribute.
volatile uint16_t* const video_memory = (uint16_t*) VIDEO_ADDRESS;

// Tracking the current cursor position.
static uint8_t terminal_row = 0;
static uint8_t terminal_column = 0;

// Scrolling the screen
void scroll_screen(void) {
    // Only scroll if we've gone beyond the last row.
    if (terminal_row < NUM_ROWS)
        return;

    // Shift all rows up one row.
    for (int i = 0; i < (NUM_ROWS - 1) * NUM_COLS; i++) {
        video_memory[i] = video_memory[i + NUM_COLS];
    }

    // Clear the last row by filling it with spaces.
    for (int i = (NUM_ROWS - 1) * NUM_COLS; i < NUM_ROWS * NUM_COLS; i++) {
        video_memory[i] = ' ' | (DEFAULT_COLOR << 8);
    }

    // Adjust the current row to the bottom line.
    terminal_row = NUM_ROWS - 1;
}


// This function will be passed to esp_printf.

void putc(int data) {
    // If newline, reset column and increment row.
    if (data == '\n') {
        terminal_column = 0;
        terminal_row++;
        scroll_screen();
        return;
    }

    // Finding the index in video memory
    uint16_t index = terminal_row * NUM_COLS + terminal_column;
    // Write the character with the default color.
    video_memory[index] = (data & 0xFF) | (DEFAULT_COLOR << 8);

    // Advance the cursor column.
    terminal_column++;

    // If we exceed the end of the row, wrap to the next row.
    if (terminal_column >= NUM_COLS) {
        terminal_column = 0;
        terminal_row++;
        scroll_screen();
    }
}

// Running the terminal driver and the print function
int main(void) {
    // This putc function will be used by rprintf and esp_printf
    int current_execution_level = 3;  // This is a demo value.
    esp_printf(putc, "Current Execution Level: %d\r\n", current_execution_level);

    // Debbugging
    /*
    for (int i = 0; i < 3000; i++) {
        putc('A');
    }
    */

    return 0;
}
