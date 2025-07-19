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

#ifdef USE_K1986VE92xI
#include "k1986be92fi.h"
#endif // USE_MDR1986VE9x

#ifndef RETARGET_TX_FUNC
#error "define RETARGET_TX_FUNC"
#endif // RETARGET_TX_FUNC

#ifndef RETARGET_TX_BUF_SIZE
#error "define RETARGET_TX_BUF_SIZE"
#endif // RETARGET_TX_BUF_SIZE 

#ifndef RETARGET_TX_BUF
#error "define RETARGET_TX_BUF"
#endif // RETARGET_TX_BUF 

/// @brief Указатель на буфер для хранения выхлопа printf.
static uint8_t* printf_buf = RETARGET_TX_BUF;

/// @brief Указатель на функцию для передачи данных.
const int (*printf_buf_transmit)(uint8_t* buf, int len) = RETARGET_TX_FUNC;

/// @brief Размер массива printf_buf.
volatile uint32_t printf_buf_size = RETARGET_TX_BUF_SIZE;

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


#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

int _close(int fd) {
    (void)fd;
    return -1; // Всегда ошибка (нет файловой системы)
}

int _fstat(int fd, struct stat *st) {
    if (fd < 3) { // STDIN/STDOUT/STDERR
        st->st_mode = S_IFCHR; // Характеризуем как терминальное устройство
        return 0;
    }
    errno = EBADF;
    return -1;
}

int _isatty(int fd) {
    return (fd < 3) ? 1 : 0; // Только стандартные потоки - терминалы
}

off_t _lseek(int fd, off_t offset, int whence) {
    (void)offset;
    (void)whence;
    errno = ESPIPE; // "Illegal seek" - для потоковых устройств
    return (off_t)-1;
}

int _getpid(void) {
    return 1; // Минимальный допустимый PID
}

int _kill(int pid, int sig) {
    (void)pid;
    (void)sig;
    errno = EPERM; // "Operation not permitted"
    return -1;
}
