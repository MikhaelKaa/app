// 
// file: vcd_log.h
// https://github.com/MikhaelKaa
// 20.01.2025

#include "vcd_log.h"
#include "print_bin_macro.h"

typedef struct vcd_log_var {
  void* var;
  vcd_var_type_t type;
  char* name;
  char identifier;
  int32_t int32_last_value;
}vcd_log_var_t;

vcd_log_var_t vars[VCD_NUM];
const char vcd_version[] = "vcd log version 0.1";
const char identifier_start = '!';
static FILE* vcd_f = NULL;
const char time_number_text[][3+1]  = {"1", "10", "100"};
const char time_unit_text[][2+1]    = {"s", "ms", "ns", "us", "ps", "fs"};
const char type_text[][7+1]         = {"wire", "real", "integer", "integer", "integer"};
const char size_text[][2+1]         = {"1", "32", "8", "16", "32"};
char identifier_cnt = 0;

// timestamp пока так.
// TODO: исправить работу с timestamp.
unsigned long int vcd_timestamp = 0;

int vcd_var_add(void* var, vcd_var_type_t type, char* name) {
  if(VCD_NUM == identifier_cnt) return -1;
  // ... и другие проверки ...

  vars[(int)identifier_cnt].type = type;
  vars[(int)identifier_cnt].name = name;
  vars[(int)identifier_cnt].identifier = identifier_cnt + identifier_start;
  vars[(int)identifier_cnt].var = var;
  identifier_cnt++;
  return 0;
}

int vcd_init(FILE* file, char* name, char* date, char* comment, vcd_time_number_t time_number, vcd_time_unit_t time_unit) {

  if(!file) return -1;
  // по хорошему надо бы все указатели проверять...
  vcd_f = file;

  // VCD header.
  fprintf(vcd_f, "$date %s $end\n", date);
  fprintf(vcd_f, "$version  %s $end\n", vcd_version);
  fprintf(vcd_f, "$comment %s $end\n", comment);
  fprintf(vcd_f, "$timescale %s %s $end\n", time_number_text[time_number], time_unit_text[time_unit]);
  // Перечисление переменных VCD файла.
  fprintf(vcd_f, "$scope module %s $end\n", name);
  for(int i = 0; i < identifier_cnt; i++) {
    fprintf(vcd_f, "$var %s %s %c %s $end\n", type_text[vars[i].type], size_text[vars[i].type], vars[i].identifier, vars[i].name);
  }
  fprintf(vcd_f, "$upscope $end\n");
  fprintf(vcd_f, "$enddefinitions $end\n");
  //
  fprintf(vcd_f, "#%lu\n", vcd_timestamp++);
  
  for(int i = 0; i < identifier_cnt; i++) {
    switch ((int)vars[i].type)
    {
    case vcd_type_bool:
      fprintf(vcd_f, "%c%c\n", (*(bool*)(vars[i].var))?'1':'0', vars[i].identifier);
      break;

    case vcd_type_real:
      fprintf(vcd_f, "r%0.15f %c\n", (*(float*)(vars[i].var)), vars[i].identifier);
      break;

    case vcd_type_int8:
      fprintf(vcd_f, "b"PRINTF_BINARY_PATTERN_INT8" %c\n", PRINTF_BYTE_TO_BINARY_INT8((*(int8_t*)(vars[i].var))), vars[i].identifier);
      break;

    case vcd_type_int16:
      fprintf(vcd_f, "b"PRINTF_BINARY_PATTERN_INT16" %c\n", PRINTF_BYTE_TO_BINARY_INT16((*(int16_t*)(vars[i].var))), vars[i].identifier);
      break;

    case vcd_type_int32:
      // if(vars[i].int32_last_value != *(int32_t*)vars[i].var) {
        fprintf(vcd_f, "b"PRINTF_BINARY_PATTERN_INT32" %c\n", \
          PRINTF_BYTE_TO_BINARY_INT32((*(int32_t*)(vars[i].var))), \
          vars[i].identifier);
        vars[i].int32_last_value = *(int32_t*)vars[i].var;
      // }
      break;

    default:
      break;
    }
  }
  return 0;
}

// TODO: доделать запись только при изменении значения остальных типов (сделан только int32).
void vcd_proc(void) {
  // TODO: запись метки времени даже если переменные не изменились - придумай что нибудь.
  // Метка времени.
  fprintf(vcd_f, "#%lu\n", vcd_timestamp++);

  for(int i = 0; i < identifier_cnt; i++) {
    switch ((int)vars[i].type)
    {
    case vcd_type_bool:
      fprintf(vcd_f, "%c%c\n", (*(bool*)(vars[i].var))?'1':'0', vars[i].identifier);
      break;

    case vcd_type_real:
      fprintf(vcd_f, "r%0.15f %c\n", (*(float*)(vars[i].var)), vars[i].identifier);
      break;

    case vcd_type_int8:
      fprintf(vcd_f, "b"PRINTF_BINARY_PATTERN_INT8" %c\n", PRINTF_BYTE_TO_BINARY_INT8((*(int8_t*)(vars[i].var))), vars[i].identifier);
      break;

    case vcd_type_int16:
      fprintf(vcd_f, "b"PRINTF_BINARY_PATTERN_INT16" %c\n", PRINTF_BYTE_TO_BINARY_INT16((*(int16_t*)(vars[i].var))), vars[i].identifier);
      break;

    case vcd_type_int32:
      if(vars[i].int32_last_value != *(int32_t*)vars[i].var) {
        fprintf(vcd_f, "b"PRINTF_BINARY_PATTERN_INT32" %c\n", \
          PRINTF_BYTE_TO_BINARY_INT32((*(int32_t*)(vars[i].var))), \
          vars[i].identifier);
        vars[i].int32_last_value = *(int32_t*)vars[i].var;
      }
      break;

    default:
      break;
    }
  }
}
