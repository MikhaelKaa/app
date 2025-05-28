// Header: retarget.c
// File Name: retarget.c
// Author: Михаил Каа
// Date: 18.04.2025

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "retarget.h"

#ifdef USE_HAL_DRIVER
#include "STM32_hal.h"
#endif // USE_HAL_DRIVER


#ifdef K1946VK035
#include "k1946vk035.h"
#endif // K1946VK035

#ifdef USE_MDR1986VE9x
#include "k1986be92fi.h"
#endif // USE_MDR1986VE9x

#ifndef PRINTF_TX_FUNC
#error "define PRINTF_TX_FUNC"
#endif // PRINTF_TX_FUNC

#ifndef USART_TX_DMA_BUF_SIZE
#error "define USART_TX_DMA_BUF_SIZE"
#endif // USART_TX_DMA_BUF_SIZE 

#ifndef PRINTF_TX_BUF
#error "define PRINTF_TX_BUF"
#endif // PRINTF_TX_BUF 

/// @brief Указатель на буфер для хранения выхлопа printf.
static uint8_t* printf_buf = PRINTF_TX_BUF;

/// @brief Указатель на функцию для передачи данных.
const int (*printf_buf_transmit)(uint8_t* buf, int len) = PRINTF_TX_FUNC;

/// @brief Размер массива printf_buf.
volatile uint32_t printf_buf_size = USART_TX_DMA_BUF_SIZE;

/// @brief Счетчик символов в буфере printf.
volatile int printf_cnt = 0U;

void printf_flush(void)
{
  int size = 0;
  int ans = 0;

  if(printf_cnt)
  {
    size = printf_cnt;
    ans = printf_buf_transmit((uint8_t*)printf_buf, size);
    if(0 != ans) return;
    printf_cnt = 0;
  }
}

int _write(int fd, char* ptr, int len)
{
  (void)fd;
  int i = 0;
  while (i < len) {
    printf_buf[printf_cnt++ % printf_buf_size] = (uint8_t)ptr[i++];
  }
  return len;
}

#ifdef __GNUC__
  /* With GCC, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  // #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
  #define GETCHAR_PROTOTYPE int __io_getchar(void)
#else
  // #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
  #define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif /* __GNUC__ */

static volatile uint32_t uart_buf_cnt_out = 0;
GETCHAR_PROTOTYPE {
  uint8_t ch = 0;
  if(uart_buf_cnt_out < uart_buf_cnt_in) {
    ch = RETARGET_RX_BUF[uart_buf_cnt_out++%RETARGET_RX_BUF_SIZE];
  }
  return ch;
}

int _read(int file, char *ptr, int len) {
  (void)file;
  for (int DataIdx = 0; DataIdx < len; DataIdx++) {
      *ptr++ = __io_getchar();
  }
  return len;
}