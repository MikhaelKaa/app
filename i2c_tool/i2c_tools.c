// i2c_tools for mcu
// Kargapolcev M. E.
// 03.03.2025
#ifdef USE_HAL_DRIVER
#include <stdio.h>
#include <stdint.h>
#include <string.h>

void i2c_scan(void);
uint8_t i2c_read(uint8_t dev, uint8_t adr);

// Dump hex to serial console
int ucmd_i2c(int argc, char **argv) {
  static uint16_t argv2 = 0;
  static uint16_t argv3 = 0;
  
  switch (argc) {
    case 1:
    printf("i2c usage: i2c scan | read dev adr reg | ...\r\n");
    break;
    
    case 2:
    if(strcmp(&argv[1][0], "scan") == 0) i2c_scan();
    break;
    
    case 4:
    if(strcmp(&argv[1][0], "read") == 0) {
      sscanf(&argv[2][0], "%hu", &argv2);
      sscanf(&argv[3][0], "%hu", &argv3);
      printf("i2c read dev %d reg %d = 0x%02x\r\n", argv2, argv3, i2c_read(argv2, argv3));
    }
    break;
    
    default:
    break;
  }
  return 0;
}

// Эта часть платформозависимая.

// TODO: оно пока прибито гвоздями, но можно сделать настраиваемым.
#include "stm32f4xx_hal.h"
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c3;

I2C_HandleTypeDef *i2c_port = &hi2c3;

void i2c_scan(void) {
  HAL_StatusTypeDef result = HAL_ERROR;
  for(int i = 0; i < 128; i++) {
    result = HAL_I2C_IsDeviceReady(i2c_port, (uint16_t)(i<<1), 2, 2);
    if(result == HAL_OK) printf("%02x ", i);
    if(result == HAL_ERROR) printf("-- ");
    if(result == HAL_TIMEOUT) printf("-t ");

    if ((i + 1) % 16 == 0) {printf("\r\n");}
  }
}

// Не очень правильно. Нужно возвращать статус операции, а результат прокидывать через указатель.
uint8_t i2c_read(uint8_t dev, uint8_t adr) {
  uint8_t res = 0;
  HAL_I2C_Mem_Read(i2c_port, dev, adr, 1, &res, 1, 100);
  return res;
}
#endif /* USE_HAL_DRIVER */
