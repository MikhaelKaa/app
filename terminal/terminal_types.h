// file: terminal_types.h
// Terminal types
// https://github.com/MikhaelKaa
// 01.07.2025

#ifndef _TERMINAL_TYPES_
#define _TERMINAL_TYPES_

#include <stdint.h>

typedef enum term_colors {
    black =   0, // Black
    red =     1, // Red
    green =   2, // Green
    yellow =  3, // Yellow
    blue =    4, // Blue
    magenta = 5, // Magenta
    cyan =    6, // Cyan
    white =   7, // White
}term_colors_t;

const term_colors_t fore_default = green;
const term_colors_t back_default = black;


// Структура терминала
typedef struct {
    // Размеры в символах
    uint16_t cols;
    uint16_t rows;
    
    // Позиция курсора в символах
    uint16_t cursor_x;
    uint16_t cursor_y;
    
    // Параметры шрифта
    uint16_t font_width;
    uint16_t font_heigh;
    
    // Параметры текущего цвета
    term_colors_t fore;
    term_colors_t back;

} terminal_t;

typedef void (*esc_handler)(terminal_t*, const char*);

typedef struct esc_seq_entry {
    const char *seq;       // Паттерн: "A", "[5n", "?25h"
    esc_handler handler;
} esc_seq_entry_t;

#endif // _TERMINAL_TYPES_

