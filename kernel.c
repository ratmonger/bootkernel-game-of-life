#include <stdint.h>

/**
 * Function: p
 * ----------------------------
 * Draws a pattern onto a 2D grid in VGA memory starting at the specified (x, y) coordinates.
 *
 * Parameters:
 *    A        - Pointer to a volatile array representing the grid (VGA buffer).
 *    pattern  - Array of characters encoding the pattern to be drawn.
 *    x        - Horizontal starting position on the grid.
 *    y        - Vertical starting position on the grid.
 *
 * Returns:
 *    None. The function modifies the contents of the array A in place.
 */

void p(volatile uint16_t* A, const char pattern[], unsigned short x, unsigned short y) {
    unsigned short  index = x + y * 80;
    uint16_t i,row = 0, count = 0;

    // Provided patterns have length lesas than 256
    for (i = 0; i < 255; i++) {
        // Decode the pattern array extracting 2 bits storing into a byte.
        char c = (pattern[i / 4] >> (6 - 2 * (i % 4))) & 0x3;

        // Byte = 2 -> Jump to next row
        if (c == 0x2) {
            row++;
            count = 0;
        }
        // Byte = 1 -> Turn Cell 'On'
        else if (c == 0x1) {
            A[row * 80 + index + count] |= (0x4 << 12);
            count++;
        }
        // Byte = 0 -> Cell remains 'Off'
        else if (!c) {
            count++;
        }
        // Byte = 3 -> End of Pattern
        else {
            break;
        }
    }
}


void dummy_test_entrypoint() {}


/**
 * Function: v
 * ----------------------------
 * Extracts a specific background colour bit from a 16-bit value of VGA memory.
 * The extracted bit denotes the cell state in Game of Life.
 *
 * Parameters:
 *    val - A 16-bit integer value from which the bit is extracted.
 *
 * Returns:
 *    uint8_t - Returns 1 if the 4 most significant bits of val equal 0x4,
 *              otherwise returns 0.
 */
uint8_t v(uint16_t val) {
    return ((val >> 12) & 0xF) == 0x4 ? 1 : 0;
}


void main() {
    // Declare VGA memory address
    volatile uint16_t* A = (volatile uint16_t*)0xb8000;

    // Text inserted into VGA memory.
    const char name[] = "Bryce PalmerGame of Life";

    // Glidergun pattern encoded into 2 bit scheme.
    const unsigned char glidergun[59] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x50, 0x00, 0x50, 0x00,
        0x00, 0x05, 0x80, 0x00, 0x00, 0x40, 0x40, 0x14, 0x00, 0x00,
        0x01, 0x65, 0x00, 0x00, 0x40, 0x04, 0x05, 0x94, 0x00, 0x01,
        0x01, 0x14, 0x01, 0x18, 0x00, 0x00, 0x10, 0x01, 0x00, 0x01,
        0x80, 0x00, 0x00, 0x40, 0x60, 0x00, 0x00, 0x05, 0xff
    };

    // Barber Pole pattern encoded into 2 bit scheme.
    const unsigned char bpole[16] = {
        0x10, 0x02, 0x40, 0x02, 0x40, 0x02, 0x15, 0x02,
        0x00, 0x02, 0x00, 0x52, 0x00, 0x46, 0x00, 0x17
    };

    unsigned short i;
    // Clears majority of the initial text written to VGA memory.
    // Replaces with spaces: ' '
    for (i = 0; i < 1000; i++) {
        A[i] = name[5];
    }

    // Define yellow background value
    // Set border cells to yellow background with red foreground
    volatile uint16_t temp = 0xE4 << 8;
    for (i = 0; i < 80; i++) {
        A[i] = temp;
        A[i + 1920] = temp;
    }
    for (i = 0; i < 25; i++) {
        A[i * 80] = temp;
        A[i * 80 + 79] = temp;
    }

    // Writes text to the top and bottom borders.
    for (i = 0; i < 12; i++) {
        A[i + 1954] |=  name[i];
        A[i + 34] |=  name[i+12];
    }


    // Write patterns to VGA memory at specified coordinates.
    p(A, glidergun, 40, 2);
    p(A, glidergun, 6, 4);
    p(A, bpole, 40, 13);
    p(A, bpole, 56, 14);
    p(A, bpole, 4, 13);
    p(A, bpole, 15, 16);


    unsigned short n;
    uint8_t dim =78,j, liveNeighbors, shift = 0;
    unsigned short count = 203;
    while (1) {
        // Add delay to game state updates.
        if (++count > 200) {
            count = 0;

            // Count neighboring cells
            for (i = 1; i < 24; i++) {
                for (j = 1; j < dim + 1; j++) {
                    n = (i * (dim + 2)) + j;
                    liveNeighbors = v(A[n - 1]) + v(A[n + 1]) + v(A[n - 80]) + v(A[n + 80])
                        + v(A[n - 81]) + v(A[n - 79]) + v(A[n + 79]) + v(A[n + 81]);

                    // Let judgement be cast upon thee, live or die.
                    // Store the NEW game state in VGA foreground bits.
                    if (v(A[n])) {
                        A[n] = (liveNeighbors < 2 || liveNeighbors > 3) ? (A[n] & ~(1 << 8)) : (A[n] | (1 << 8));
                    } else {
                        A[n] = (liveNeighbors == 3) ? (A[n] | (1 << 8)) : (A[n] & ~(1 << 8));
                    }
                }
            }

        }
        // Update the VGA background bits with the NEW state from the foreground bits.
        for (i = 1; i < 24; i++) {
            for (j = 1; j < dim + 1; j++) {
                n = (i * (dim + 2)) + j;
                A[n] = (A[n] & ~(0xF << 12)) | (((A[n] >> 8) & 0x1) << 14);
            }
        }

    }
}

