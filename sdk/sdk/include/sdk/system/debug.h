#ifndef _SDK_SYSTEM_DEBUG_H
#define _SDK_SYSTEM_DEBUG_H
#include <stdbool.h>

/**
 * Returns the current position of the cursor in debug text mode.
 * 
 * @param x Pointer to an int to store the X position of the cursor in.
 * @param y Pointer to an int to store the Y position of the cursor in.
 * @return Always returns 0.
 */
int Debug_GetCursorPosition(int *x, int *y);

/**
 * Waits until a key is pressed, then returns a number representing the key.
 * Only appears to react to number keys and the Power/Clear key. Returns 0x30
 * to 0x39 for keys 0 to 9, and 0x98 for the Power/Clear key.
 * 
 * @return A number representing the key that was pressed.
 */
int Debug_GetKey();

/**
 * Prints a string in debug text mode, either in normal black-on-white or
 * inverted white-on-black. Color inversion occurs if \c invert is true.
 * 
 * Returns false if the string did not fit on the screen.
 * 
 * @param string The string to print.
 * @param invert True if the colors used to print the text should be inverted.
 * @return True if writing the string was successful, false otherwise.
 */
bool Debug_PrintString(char *string, bool invert);

/**
 * Sets the position of the cursor in debug text mode.
 * 
 * @param x The cursor's new X position.
 * @param y The cursor's new Y position.
 * @return Always returns 0.
 */
int Debug_SetCursorPosition(int x, int y);

#endif