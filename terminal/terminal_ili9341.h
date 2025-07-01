// Terminal with esc-seq
// file: terminal_ili9341.h
// https://github.com/MikhaelKaa
// 28.06.2025

#ifndef _TERMINAL_ILI9341_
#define _TERMINAL_ILI9341_

#include <stdint.h>

// screen draw char functions
// void term_draw_char(uint16_t x, uint16_t y, char c);
void term_draw_char(uint16_t x, uint16_t y, char c, uint16_t fore, uint16_t back);
void term_set_font(uint16_t font);
void term_set_back_color(uint16_t color);
void term_set_fore_color(uint16_t color);
void term_screen_fill(uint16_t color);
void term_screen_scroll(uint16_t pix);

#endif // _TERMINAL_ILI9341_

