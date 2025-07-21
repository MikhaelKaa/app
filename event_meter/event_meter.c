/**
 * @file event_meter.c
 * @brief Для измерения времени между событиями.
 * @author Mikhael Kaa (Михаил Каа)
 * @date 21.07.2025
 */
#include <limits.h>
#include <stdio.h>

#include "event_meter.h"

#ifndef ENDL
#define ENDL "\r\n"
#endif // ENDL

void event_meter_show(event_meter_t* meter) {

    if(meter->event_diff_cnt < EVENT_METER_BUF_LEN) {
        printf("%s: no measurements yet" ENDL, meter->name);
        return;
    }

    uint32_t _max = 0;
    uint32_t _min = UINT32_MAX;
    uint64_t _avg = 0;

    for(uint32_t i = 0; i < EVENT_METER_BUF_LEN; i++) {
        uint32_t val = meter->event_time_diff[i];
        if(val > _max) _max = val;
        if(val < _min) _min = val;
        _avg += val;
    }
    _avg /= EVENT_METER_BUF_LEN;

    printf("%s max = %lu " EVENT_METER_TIME_UNIT ENDL, meter->name, _max);
    printf("%s min = %lu " EVENT_METER_TIME_UNIT ENDL, meter->name, _min);
    printf("%s avg = %lu " EVENT_METER_TIME_UNIT ENDL, meter->name, (uint32_t)_avg);
}

#undef ENDL
