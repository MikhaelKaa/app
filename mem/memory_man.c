// memory_man.c
// Утилиты работы с памятью.
// Михаил Каа
// 01.03.2025

#include <stdio.h>
#include <stdint.h>
#include <string.h>

void mem_dump(uint8_t *buf, uint32_t len);
uint32_t mem_test(uint8_t *buf, uint32_t len);

// ucmd handler for mem_dump.
int ucmd_mem(int argc, char *argv[])
{
  static uint32_t argv2 = 0;
  static uint32_t argv3 = 0;
  static uint32_t argv4 = 0;
  // static uint16_t argv5 = 0;

  switch (argc) {
    case 1:
    case 2:
    printf("mem usage: mem dump adr len | read adr | write adr data |\r\n");
    printf("\tcpy dst src len\r\n");
      
      break;

    case 3:
      // mem read adr
      if(strcmp(&argv[1][0], "read") == 0) {
        sscanf(&argv[2][0], "%lx", &argv2);
        printf("0x%02x\r\n", *(uint8_t*)argv2);
      }
      break;
    case 4:
      // mem test adr len
      if(strcmp(&argv[1][0], "test") == 0) {
        sscanf(&argv[2][0], "%lx", &argv2); // adr
        sscanf(&argv[3][0], "%lx", &argv3); // len
        mem_test((uint8_t*)argv2, argv3);
      }
      

      // mem dump adr len
      if(strcmp(&argv[1][0], "dump") == 0) {
        sscanf(&argv[2][0], "%lx", &argv2); // adr
        sscanf(&argv[3][0], "%lx", &argv3); // len
        mem_dump((uint8_t*)argv2, argv3);
      }
    
      // mem write adr data
      if(strcmp(&argv[1][0], "write") == 0) {
        sscanf(&argv[2][0], "%lx", &argv2); // adr
        sscanf(&argv[3][0], "%lx", &argv3); // data
        *(uint8_t*)argv2 = argv3;
      }
      break;
    case 5:
      // mem copy
      if(strcmp(&argv[1][0], "cpy") == 0) {
        sscanf(&argv[2][0], "%lx", &argv2); // dst
        sscanf(&argv[3][0], "%lx", &argv3); // src
        sscanf(&argv[4][0], "%lx", &argv4); // len
        memcpy((void*)argv2, (void*)argv3, argv4);
      }
      
      break;
    default:
      break;
    }
  return 0;
}

// Dump memory at hex & ascii to serial console
void mem_dump(uint8_t *buf, uint32_t len) {
    uint32_t i = 0;
    uint32_t start = (uint32_t)buf;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 0) {printf("0x%08lx ", start);}
        printf("%02x ", buf[i]);
        // Каждые 16 значений.
        if ((i + 1) % 16 == 0) {
          // Вывод ascii символов.
          printf(" ");
          for(int32_t n = 15; n >= 0; n--) {
            uint8_t c = buf[i-n];
            if(c >= 32 && c <= 126) printf("%c", c);
            else printf(".");
          }
          printf("\r\n");
        }
        ++start;
    }
    printf("\r\n");
}

uint32_t mem_test(uint8_t *buf, uint32_t len) {
  uint32_t error_count = 0;
  volatile uint8_t *b = buf;
  const uint8_t patterns[] = {0x00, 0x55, 0xAA, 0xFF};
  const int num_patterns = sizeof(patterns)/sizeof(patterns[0]);
  const uint32_t max_printf_msg = 10;

  if (len == 0) {
      printf("Zero-length test skipped\r\n");
      return 0;
  }

  for (uint32_t i = 0; i < len; i++) {
    uint8_t original = b[i];

    // Тестируем все шаблоны
    for (int p = 0; p < num_patterns; p++) {
      uint8_t test_val = patterns[p];
      b[i] = test_val;
      
      if (b[i] != test_val) {
        if(error_count < max_printf_msg) printf("ERROR @ 0x%08lx: Wrote 0x%02X, Read 0x%02X\r\n", 
          (uint32_t)(b + i), test_val, b[i]);
        error_count++;
      }
    }

    // Восстанавливаем значение с проверкой
    b[i] = original;
    if (b[i] != original) {
      if(error_count < max_printf_msg) printf("RESTORE ERROR @ 0x%08lx! Original: 0x%02X, Current: 0x%02X\r\n",
        (uint32_t)(b + i), original, b[i]);
      error_count++;
    }

    // // Индикация прогресса
    // if ((i % 8192) == 0) {
    //   printf("Tested %lu/%lu bytes (errors: %lu)\r\n", i, len, error_count);
    // }
  }

  // Итоговый отчет
  if (error_count == 0) {
    printf("Memory test PASSED: %lu bytes\r\n", len);
  } else {
    printf("Memory test FAILED! Errors: %lu/%lu bytes\r\n", error_count, len);
  }
  
  return error_count;
}
