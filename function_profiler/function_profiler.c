
/**
 * @file function_profiler.c
 * @brief Профайлер для измерения времени выполнения функций
 * @author Нейросетка
 * @date 21.07.2025
 */
#include <stdio.h>
#include <limits.h>
#include "function_profiler.h"

#ifndef ENDL
#define ENDL "\r\n"
#endif

void function_profiler_reset(function_profiler_t* profiler) {
    profiler->total_time = 0;
    profiler->max_time = 0;
    profiler->min_time = UINT32_MAX;
    profiler->call_count = 0;
    profiler->buffer_index = 0;
    
    for (int i = 0; i < PROFILER_BUF_LEN; i++) {
        profiler->time_buffer[i] = 0;
    }
}

bool function_profiler_get_stats(function_profiler_t* profiler, 
                                uint32_t* min, 
                                uint32_t* max, 
                                uint32_t* avg) {
    if (profiler->call_count == 0) {
        return false;
    }

    if (min) *min = profiler->min_time;
    if (max) *max = profiler->max_time;
    
    // Точное вычисление среднего
    if (avg) {
        *avg = (uint32_t)((profiler->total_time * 1000000ULL) / SystemCoreClock / profiler->call_count);
    }
    
    return true;
}

void function_profiler_show(function_profiler_t* profiler) {
    if (profiler->call_count == 0) {
        printf("%s: no measurements yet" ENDL, profiler->name);
        return;
    }

    uint32_t min, max, avg;
    function_profiler_get_stats(profiler, &min, &max, &avg);
    
    // Точное преобразование общего времени
    uint64_t total_us = (profiler->total_time * 1000000ULL) / SystemCoreClock;
    
    printf("Function profiler: %s" ENDL, profiler->name);
    printf("  Calls: %lu" ENDL, (unsigned long)profiler->call_count);
    printf("  Total: %lu " PROFILER_TIME_UNIT ENDL, (unsigned long)total_us); // Исправленный формат
    printf("  Max:   %lu " PROFILER_TIME_UNIT ENDL, (unsigned long)max);
    printf("  Min:   %lu " PROFILER_TIME_UNIT ENDL, (unsigned long)min);
    printf("  Avg:   %lu " PROFILER_TIME_UNIT ENDL, (unsigned long)avg);
    
    // Вывод последних измерений
    if (profiler->call_count > 0) {
        uint32_t count = (profiler->call_count < PROFILER_BUF_LEN) ? 
                         profiler->call_count : PROFILER_BUF_LEN;
        
        printf("  Last %lu measurements: ", count);
        
        uint32_t start_idx = (profiler->buffer_index + PROFILER_BUF_LEN - count) % PROFILER_BUF_LEN;
        
        for (uint32_t i = 0; i < count; i++) {
            uint32_t idx = (start_idx + i) % PROFILER_BUF_LEN;
            printf("%lu ", (unsigned long)profiler->time_buffer[idx]);
        }
        printf(ENDL);
    }
}

#undef ENDL
