// 
// file: vcd_log.h
// https://github.com/MikhaelKaa
// 20.01.2025


#ifndef __VCD_LOG__
#define __VCD_LOG__

// #include "main.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "print_bin_macro.h"

#ifdef __cplusplus
extern "C" {
#endif

// Максимальное количество добавляемых переменных. (Не выделяем память динамически)
#define VCD_NUM (10)

typedef enum {
    vcd_type_bool = 0,
    vcd_type_real,
    vcd_type_int8,
    vcd_type_int16,
    vcd_type_int32,
}vcd_var_type_t;

typedef enum {
    vcd_time_number_1 = 0,
    vcd_time_number_10,
    vcd_time_number_100,
}vcd_time_number_t;

typedef enum {
    vcd_time_unit_s = 0,
    vcd_time_unit_ms,
    vcd_time_unit_us,
    vcd_time_unit_ns,
    vcd_time_unit_ps,
    vcd_time_unit_fs, // 
}vcd_time_unit_t;

// 
int vcd_var_add(void* var, vcd_var_type_t type, char* name);

// 
int vcd_init(FILE* file, char* name, char* date, char* comment, vcd_time_number_t time_number, vcd_time_unit_t time_unit);

// 
void vcd_proc(void);

#ifdef __cplusplus
}
#endif

#endif /* __VCD_LOG__ */

// https://web.archive.org/web/20120126073426/http://www.beyondttl.com/vcd.php
