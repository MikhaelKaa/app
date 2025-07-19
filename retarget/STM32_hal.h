// Header: реализация printf scanf для stm32 HAL.
// File Name: STM32_hal.h
// Author: Михаил Каа
// Date: 19.07.2025

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#if defined(STM32F429xx) || defined(STM32F401xC) || defined(STM32F407xx)
#include "stm32f4xx_hal.h"
#endif 

#if defined(STM32H743xx)
#include "stm32h7xx_hal.h"
#endif 

// По умолчанию используется uart1
#ifndef REATRGET_UART
#define REATRGET_UART huart1
#define REATRGET_UART_INST USART1
#endif // REATRGET_UART

#ifndef REATRGET_UART_INST
#error "define REATRGET_UART_INST in makefile or anywhere"
#endif //REATRGET_UART_INST

// Это необходимо, если для работы DMA нужно чтобы буфер был в определенной памяти.
#ifdef RETARGET_UART_RAM_PRE
#define RAM_D1 __attribute__ ((section(".RAM_D1_buf"), used)) 
#else
#define RETARGET_UART_RAM_PRE
#endif // RETARGET_UART_RAM_PRE

#ifndef RETARGET_TX_BUF_SIZE
#define RETARGET_TX_BUF_SIZE (4096U)
#endif // RETARGET_TX_BUF_SIZE

#ifndef RETARGET_RX_BUF_SIZE
#define RETARGET_RX_BUF_SIZE (1024U)
#endif // RETARGET_RX_BUF_SIZE

#define RETARGET_TX_BUF tx_buf
#define RETARGET_RX_BUF rx_buf
#define RETARGET_TX_FUNC stm_uart_tx

RETARGET_UART_RAM_PRE uint8_t uart_dma_buf[2] = {0};
RETARGET_UART_RAM_PRE uint8_t tx_buf[RETARGET_TX_BUF_SIZE] = {0};
RETARGET_UART_RAM_PRE static volatile uint8_t rx_buf[RETARGET_RX_BUF_SIZE] = {0};

// TODO: комплементарная переменная в дугом файле, надо сделать единообразно.
volatile uint32_t uart_buf_cnt_in = 0;

// HAL uart device
extern UART_HandleTypeDef REATRGET_UART;

void printf_init(void) {
  // TODO: почистить ненужное
  // memset((void *)RETARGET_TX_BUF, 0U, RETARGET_TX_BUF_SIZE);
  // __HAL_UART_ENABLE_IT(&REATRGET_UART, UART_IT_IDLE);

  HAL_UART_Receive_DMA(&REATRGET_UART, uart_dma_buf, sizeof(uart_dma_buf));
  setvbuf(stdin, NULL, _IONBF, 0);
}

static inline int stm_uart_tx(uint8_t* buf, int len)
{
    return HAL_UART_Transmit_DMA(&REATRGET_UART, buf, len);
}

static inline void u1_rx_callback(uint8_t data) {
  RETARGET_RX_BUF[uart_buf_cnt_in++%RETARGET_RX_BUF_SIZE] = data;
}

// В случае, если HAL_UART_Callback нужны еще гдето, то не определяем HAL_UART_xCallback и нужные функции реализуем самостоятельно вне этого модуля.
#ifdef HAL_UART_xCallback
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == REATRGET_UART_INST) {
    CLEAR_BIT(huart->Instance->CR2, (USART_CR2_LINEN  | USART_CR2_CLKEN)); 
    CLEAR_BIT(huart->Instance->CR3, (USART_CR3_SCEN   |\
                                     USART_CR3_HDSEL  | USART_CR3_IREN)); 
    huart->ErrorCode  = HAL_UART_ERROR_NONE; 
    huart->gState     = HAL_UART_STATE_READY; 
    huart->RxState    = HAL_UART_STATE_READY; 

    // __HAL_UART_ENABLE_IT(&REATRGET_UART, UART_IT_IDLE);
    HAL_UART_Receive_DMA(&REATRGET_UART, uart_dma_buf, sizeof(uart_dma_buf));
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == REATRGET_UART_INST) {
    u1_rx_callback(uart_dma_buf[1]);
  }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == REATRGET_UART_INST) {
    u1_rx_callback(uart_dma_buf[0]);
  }
}
#endif // HAL_UART_xCallback
