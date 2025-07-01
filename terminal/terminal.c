// file: terminal.c
// https://github.com/MikhaelKaa
// 01.07.2025

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "terminal.h"
#include "terminal_ili9341.h"
#include "terminal_types.h"

// Состояния парсера
typedef enum {
    STATE_NORMAL,   // Обычный режим (печать символов)
    STATE_ESCAPE,   // Обработка ESC-последовательности
} parser_state_t;

// Статический экземпляр терминала
static terminal_t term = {
    .cols =         24,
    .rows =         12,
    .font_width =   7,
    .font_heigh =   10,
    .cursor_x =     0,
    .cursor_y =     0,
    .back =         back_default,
    .fore =         fore_default,
};

// Состояние парсера
static parser_state_t parser_state = STATE_NORMAL;
static char esc_buffer[16];
static uint8_t esc_index = 0;

// Прототипы внутренних функций
static void clear_screen  (terminal_t*, const char*);
static void set_color     (terminal_t*, const char*);
static void cursor_set    (terminal_t*, const char*);
static void handle_char   (terminal_t*, char);
static bool esc_seq_match (const char* pattern, const char* seq);
static void handle_control_char(terminal_t* t, char c);

// Таблица ESC-последовательностей
const esc_seq_entry_t esc_table[] = {
    {"[2J", clear_screen},          // Очистка экрана
    {"[*m", set_color},             // Установка цвета
    {"[*H", cursor_set},            // Курсор
    {NULL, NULL}                    // Terminator
};

// Обработка входных данных
void terminal_input_data(char *data, uint32_t len) {
    for(uint32_t i = 0; i < len; i++) {
        char c = data[i];
        
        if(parser_state == STATE_NORMAL) {
            if(c == 0x1B) { // Начало ESC-последовательности
                parser_state = STATE_ESCAPE;
                esc_index = 0;
                esc_buffer[0] = '\0';
            }
            else if(c >= 0x20 && c <= 0x7E) {
                handle_char(&term, c);
            }
            // Обработка других управляющих символов
            else if(c == '\n' || c == '\r' || c == '\b') {
                handle_control_char(&term, c);
            }
            // Добавлена обработка других управляющих символов
            else {
                // Игнорируем или обрабатываем другие символы
            }
        }
        else if(parser_state == STATE_ESCAPE) {
            // Защита от переполнения буфера
            if(esc_index < sizeof(esc_buffer) - 1) {
                esc_buffer[esc_index++] = c;
                esc_buffer[esc_index] = '\0';
                
                // Проверяем, является ли символ завершающим
                if((c >= 'A' && c <= 'Z') || \
                   (c >= 'a' && c <= 'z') || \
                   (c == '~') || (esc_index >= sizeof(esc_buffer) - 1)) {
                    // Ищем обработчик для последовательности
                    const esc_seq_entry_t *entry = esc_table;
                    bool found = false;
                    while(entry->seq != NULL) {
                        if(esc_seq_match(entry->seq, esc_buffer)) {
                            entry->handler(&term, esc_buffer);
                            found = true;
                            break;
                        }
                        entry++;
                    }
                    
                    // Если последовательность не найдена, можно добавить лог
                    if(!found) {
                        // printf("Unknown ESC seq: %s\n", esc_buffer);
                    }
                    parser_state = STATE_NORMAL;
                }
            }
            else {
                // Переполнение буфера - сброс состояния
                parser_state = STATE_NORMAL;
            }
        }
    }
}

// Функция сравнения ESC-последовательности с шаблоном
static bool esc_seq_match(const char* pattern, const char* seq) {
    const char *p = pattern;
    const char *s = seq;

    while (*p && *s) {
        if (*p == '*') {
            // Wildcard: пропускаем любые символы до следующего символа в шаблоне
            p++;
            if (!*p) return true; // Wildcard в конце
            
            // Ищем следующий символ из шаблона в последовательности
            while (*s && *s != *p) s++;
            if (!*s) return false;
        }
        else if (*p != *s) {
            return false;
        }
        else {
            p++;
            s++;
        }
    }
    
    // Оба должны достичь конца
    return (*p == '\0' && *s == '\0');
}

// Обработка управляющих символов
static void handle_control_char(terminal_t* t, char c) {
    switch(c) {
        case '\n': // Новая строка
            t->cursor_x = 0;
            t->cursor_y++;
            if(t->cursor_y >= t->rows) {
                t->cursor_y = t->rows - 1;
                // Здесь будет вызов функции скроллинга
                // term_scroll();
            }
            break;
            
        case '\r': // Возврат каретки
            t->cursor_x = 0;
            break;
            
        case '\b': // Backspace
            if(t->cursor_x > 0) {
                t->cursor_x--;
                // Очищаем предыдущий символ
                term_draw_char(
                    t->cursor_x * t->font_width,
                    t->cursor_y * t->font_heigh,
                    ' ',
                    t->fore,
                    t->back
                );
            }
            break;
    }
}

// Обработка обычных символов
static void handle_char(terminal_t* t, char c) {
    // Рисуем символ на текущей позиции
    term_draw_char(
        t->cursor_x * t->font_width, 
        t->cursor_y * t->font_heigh, 
        c,
        t->fore,
        t->back
    );
    
    // Перемещаем курсор
    t->cursor_x++;
    if(t->cursor_x >= t->cols) {
        t->cursor_x = 0;
        t->cursor_y++;
        if(t->cursor_y >= t->rows) {
            t->cursor_y = t->rows - 1;
            // Здесь будет вызов функции прокрутки
            // term_scroll();
        }
    }
}

// Очистка экрана
static void clear_screen(terminal_t* t, const char* data) {
    (void)data; // Неиспользуемый параметр
    
    // Заливаем весь экран цветом фона
    term_screen_fill(t->back);

    // Курсор в начало
    t->cursor_x = 0;
    t->cursor_y = 0;
}

// Установка цвета
static void set_color(terminal_t* t, const char* data) {
    // data в формате SGR (Select Graphic Rendition): "[<code>;<code>...m"
    const char *ptr = data;
    
    // Пропускаем начальный '[' если он есть
    if(*ptr == '[') ptr++;
    
    // Обрабатываем все коды в последовательности
    while(*ptr && *ptr != 'm') {
        // Пропускаем разделители
        if(*ptr == ';') {
            ptr++;
            continue;
        }
        
        // Парсим числовой код
        uint16_t code = 0;
        while(*ptr >= '0' && *ptr <= '9') {
            code = code * 10 + (*ptr - '0');
            ptr++;
        }
        
        // Обрабатываем код
        switch(code) {
            case 0:  // Сброс всех атрибутов
                t->fore = fore_default;
                t->back = back_default;
                break;
                
            // Цвета текста (30-37)
            case 30 ... 37:
                t->fore = code - 30;
                break;
                
            // Сброс цвета текста
            case 39:
                t->fore = green;
                break;
                
            // Цвета фона (40-47)
            case 40 ... 47:
                t->back = code - 40;
                break;
                
            // Сброс цвета фона
            case 49:
                t->back = back_default;
                break;
        }
    }
}

// Установка позиции курсора
static void cursor_set(terminal_t* t, const char* data) {
    // data в формате "[...H", где ... может быть:
    //   пустым: [H
    //   одним числом: [5H
    //   двумя числами через точку с запятой: [10;20H
    
    const char *ptr = data;
    
    // Пропускаем начальный '[' если он есть
    if(*ptr == '[') ptr++;
    
    // Значения по умолчанию
    int row = 1; // строка (1-based)
    int col = 1; // столбец (1-based)
    
    // Парсим первое число (если есть)
    if (*ptr >= '0' && *ptr <= '9') {
        row = 0;
        while(*ptr >= '0' && *ptr <= '9') {
            row = row * 10 + (*ptr - '0');
            ptr++;
        }
    }
    
    // Если встретили разделитель, парсим второе число
    if(*ptr == ';') {
        ptr++; // пропускаем разделитель
        
        if (*ptr >= '0' && *ptr <= '9') {
            col = 0;
            while(*ptr >= '0' && *ptr <= '9') {
                col = col * 10 + (*ptr - '0');
                ptr++;
            }
        }
    }
    
    // Если указано только одно число, интерпретируем как столбец
    if (row > 0 && col == 1) {
        col = row;
        row = 1;
    }
    
    // Преобразуем 1-based координаты в 0-based индексы
    t->cursor_y = (row > 0) ? (row - 1) : 0;
    t->cursor_x = (col > 0) ? (col - 1) : 0;
    
    // Ограничиваем координаты размерами экрана
    if(t->cursor_y >= t->rows) t->cursor_y = t->rows - 1;
    if(t->cursor_x >= t->cols) t->cursor_x = t->cols - 1;
}
