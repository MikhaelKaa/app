/**
 * @file event_meter.c
 * @brief Для измерения времени между событиями.
 * @author Mikhael Kaa (Михаил Каа)
 * @date 21.07.2025
 */
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>

#include "event_meter.h"

#ifndef ENDL
#define ENDL "\r\n"
#endif // ENDL

bool event_meter_get_stats(event_meter_t* meter, uint32_t* min, uint32_t* max, uint32_t* avg) {
    if(meter->event_diff_cnt < EVENT_METER_BUF_LEN) {
        return false;
    }

    uint32_t _max = 0;
    uint32_t _min = UINT32_MAX;
    uint64_t _avg = 0;

    // Вычисляем min/max/avg только если запрошены
    bool need_min = (min != NULL);
    bool need_max = (max != NULL);
    bool need_avg = (avg != NULL);

    for(uint32_t i = 0; i < EVENT_METER_BUF_LEN; i++) {
        uint32_t val = meter->event_time_diff[i];
        
        if(need_max && (val > _max)) _max = val;
        if(need_min && (val < _min)) _min = val;
        if(need_avg) _avg += val;
    }

    if(need_avg) _avg /= EVENT_METER_BUF_LEN;

    // Возвращаем запрошенные значения
    if(min) *min = _min;
    if(max) *max = _max;
    if(avg) *avg = (uint32_t)_avg;

    return true;
}

void event_meter_show(event_meter_t* meter) {
    uint32_t min, max, avg;
    
    if(event_meter_get_stats(meter, &min, &max, &avg)) {
        printf("%s max = %lu " EVENT_METER_TIME_UNIT ENDL, meter->name, max);
        printf("%s min = %lu " EVENT_METER_TIME_UNIT ENDL, meter->name, min);
        printf("%s avg = %lu " EVENT_METER_TIME_UNIT ENDL, meter->name, avg);
    } else {
        printf("%s: no measurements yet" ENDL, meter->name);
    }
}

#undef ENDL
