#ifndef MONITOR_H

#define MONITOR_H

#include "common.h"

/*

======

COLORS

======

0	Black

1	Blue

2	Green

3	Cyan

4	Red

5	Magenta

6	Brown

7	Light gray

8	Dark gray

9	Light blue

10	Light green

11	Light cyan

12	Pink

13	Light magenta

14	Yellow

15	White

*/

// Writes a single character out to the screen.

void monitor_color_put(char c, u8int backColour, u8int foreColour);

void monitor_put(char c);

// Clears the screen, by copying lots of spaces to the framebuffer.

void monitor_clear();

// Outputs a null-terminated ASCII string to the monitor.

void monitor_write(const char *c);

void monitor_write_hex(u32int n);

void monitor_write_dec(u32int n);

// Color outputs a null-terminated ASCII string to the monitor.

void monitor_color_write(const char *c, u8int backColour, u8int foreColour);

void monitor_color_write_hex(u32int n, u8int backColour, u8int foreColour);

void monitor_color_write_dec(u32int n, u8int backColour, u8int foreColour);

#endif // MONITOR_H
