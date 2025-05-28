// Header: реализация printf scanf для stm32 HAL.
// File Name: STM32_hal.h
// Author: Михаил Каа
// Date: 28.05.2025

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifdef STM32F429xx
#include "stm32f4xx_hal.h"
#endif 
extern UART_HandleTypeDef huart1;


#define USART_TX_DMA_BUF_SIZE (512U)
uint8_t u0_tx_buf[USART_TX_DMA_BUF_SIZE];
#define PRINTF_TX_BUF u0_tx_buf

#define RETARGET_RX_BUF_SIZE (256U)
static volatile uint8_t rx_buf[RETARGET_RX_BUF_SIZE] = {0};
static volatile uint32_t uart_buf_cnt_in = 0;
#define RETARGET_RX_BUF rx_buf


void printf_init(void) {
    memset((void *)PRINTF_TX_BUF, 0U, USART_TX_DMA_BUF_SIZE);

    // setvbuf(stdin, NULL, _IONBF, 0);
}


#define PRINTF_TX_FUNC stm_uart_tx
static inline int stm_uart_tx(uint8_t* buf, int len)
{
    return HAL_UART_Transmit_DMA(&huart1, buf, len);
}

