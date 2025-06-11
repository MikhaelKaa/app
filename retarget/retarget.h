// Header: retarget.h
// File Name: retarget.h
// Author: Михаил Каа
// Date: 18.04.2025

#ifndef RETARGET
#define RETARGET

/// @brief Инициализация отправки выхлопа printf
/// @param  
void printf_init(void);

/// @brief Отправка накопленного буфера printf.
/// @param  
void printf_flush(void);

#ifndef HAL_UART_xCallback
extern uint8_t uart_dma_buf[2];
void u1_rx_callback(uint8_t data);
#endif

#endif /* RETARGET */
