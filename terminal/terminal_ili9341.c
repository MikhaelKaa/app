// Terminal with esc-seq. ili9341 implementtation
// file: terminal_ili9341.c
// https://github.com/MikhaelKaa
// 01.07.2025

#include "ili9341.h"
#include "fonts.h"

// FontDef * font = &Font_7x10;

// Цветовая палитра (R5G6B5)
static const uint16_t color_table[] = {
    0x0000, // Black
    0xF800, // Red
    0x07E0, // Green
    0xFFE0, // Yellow
    0x001F, // Blue
    0xF81F, // Magenta
    0x07FF, // Cyan
    0xFFFF, // White
};

uint16_t back_color = color_table[0];
uint16_t fore_color = color_table[2];

// screen draw char functions
void term_draw_char(uint16_t x, uint16_t y, char c, uint16_t fore, uint16_t back) {
    ILI9341_Select();
    ILI9341_WriteChar(x, y, c, Font_7x10, color_table[fore], color_table[back]);
    ILI9341_Unselect();
}

void term_set_font(uint16_t font) {

}

void term_set_back_color(uint16_t color) {
    back_color = color_table[color];
}

void term_set_fore_color(uint16_t color) {
    fore_color = color_table[color];
}

void term_screen_fill(uint16_t color) {
    ILI9341_FillScreen(back_color);
}

void term_screen_scroll(uint16_t pix) {

}


