/**
 * @file event_meter.h
 * @brief Для измерения времени между событиями.
 * @author Mikhael Kaa (Михаил Каа)
 * @date 21.07.2025
 */

#ifndef EVENT_METER_H
#define EVENT_METER_H

#include <stdint.h>

#ifndef EVENT_METER_BUF_LEN
#define EVENT_METER_BUF_LEN 16
#endif // EVENT_METER_BUF_LEN

#ifndef EVENT_METER_TIME_UNIT
#define EVENT_METER_TIME_UNIT "us"
#endif // EVENT_METER_TIME_UNIT

typedef struct {
    uint32_t event_time_last;
    uint32_t event_diff_cnt;
    uint32_t event_time_diff[EVENT_METER_BUF_LEN];
    char name[16];
} event_meter_t;

// Макрос инициализации.
// Пример использования:
// event_meter_t vsync = EVENT_METER_INIT("vsync");
#define EVENT_METER_INIT(name) {0, 0, {0}, name}

// Функция замера времени.
// Для корректной работы должна быть вызвана больше чем EVENT_METER_BUF_LEN (по умолчанию 16) раз.
static inline void event_meter_record(event_meter_t* meter, uint32_t current_time) {
    meter->event_time_diff[meter->event_diff_cnt % EVENT_METER_BUF_LEN] = current_time - meter->event_time_last;
    meter->event_time_last = current_time;
    meter->event_diff_cnt++;
}

// Вывод статистики
void event_meter_show(event_meter_t* meter);

#endif // EVENT_METER_H
