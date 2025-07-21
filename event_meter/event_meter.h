/**
 * @file event_meter.h
 * @brief Для измерения времени между событиями.
 * @author Mikhael Kaa (Михаил Каа)
 * @date 21.07.2025
 */

#ifndef EVENT_METER_H
#define EVENT_METER_H

#include <stdint.h>
#include <stdbool.h>

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

/**
 * @brief Получение статистических данных
 * @param meter Указатель на измеритель
 * @param min Указатель для минимального значения (может быть NULL)
 * @param max Указатель для максимального значения (может быть NULL)
 * @param avg Указатель для среднего значения (может быть NULL)
 * @return true - статистика доступна, false - недостаточно измерений
 * 
 * @note Если параметр min/max/avg равен NULL, соответствующее значение не вычисляется
 */
bool event_meter_get_stats(event_meter_t* meter, uint32_t* min, uint32_t* max, uint32_t* avg);

#endif // EVENT_METER_H
