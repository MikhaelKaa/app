/**
 * @file function_profiler.h
 * @brief Профайлер для измерения времени выполнения функций
 * @author Нейросетка
 * @date 21.07.2025
 */

#ifndef FUNCTION_PROFILER_H
#define FUNCTION_PROFILER_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32h7xx_hal.h"

extern uint32_t SystemCoreClock;

#ifndef PROFILER_BUF_LEN
#define PROFILER_BUF_LEN 16
#endif

#ifndef PROFILER_TIME_UNIT
#define PROFILER_TIME_UNIT "us"
#endif

typedef struct {
    uint64_t total_time;
    uint32_t max_time;
    uint32_t min_time;
    uint32_t call_count;
    uint32_t time_buffer[PROFILER_BUF_LEN];
    uint32_t buffer_index;
    char name[16];
} function_profiler_t;

#define FUNCTION_PROFILER_INIT(name) {0, 0, UINT32_MAX, 0, {0}, 0, name}

static inline uint64_t function_profiler_start(void) {
    return DWT->CYCCNT;
}

static inline void function_profiler_stop(function_profiler_t* profiler, uint64_t start_cycles) {
    uint64_t end_cycles = DWT->CYCCNT;
    uint64_t cycles;
    
    // Обработка переполнения 32-битного счетчика
    if (end_cycles >= start_cycles) {
        cycles = end_cycles - start_cycles;
    } else {
        cycles = (0x100000000ULL + end_cycles) - start_cycles;
    }
    
    // Точное преобразование циклов в микросекунды
    uint32_t duration = (uint32_t)((cycles * 1000000ULL) / SystemCoreClock);
    
    // Обновление статистики
    profiler->total_time += cycles;
    profiler->call_count++;
    
    if (duration > profiler->max_time) profiler->max_time = duration;
    if (duration < profiler->min_time) profiler->min_time = duration;
    
    // Сохранение в буфере
    profiler->time_buffer[profiler->buffer_index] = duration;
    profiler->buffer_index = (profiler->buffer_index + 1) % PROFILER_BUF_LEN;
}

void function_profiler_reset(function_profiler_t* profiler);
void function_profiler_show(function_profiler_t* profiler);
bool function_profiler_get_stats(function_profiler_t* profiler, 
                                uint32_t* min, 
                                uint32_t* max, 
                                uint32_t* avg);

#endif
