// Header: реализация printf scanf для stm32 HAL.
// File Name: STM32_hal.h
// Author: Михаил Каа
// Date: 28.05.2025

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#if defined(STM32F429xx) || defined(STM32F401xC)
#include "stm32f4xx_hal.h"
#endif 
extern UART_HandleTypeDef huart1;


#define USART_TX_DMA_BUF_SIZE (4096U)
uint8_t u0_tx_buf[USART_TX_DMA_BUF_SIZE];
#define PRINTF_TX_BUF u0_tx_buf

uint8_t uart_dma_buf[2];

#define RETARGET_RX_BUF_SIZE (1024U)
static volatile uint8_t rx_buf[RETARGET_RX_BUF_SIZE] = {0};
#define RETARGET_RX_BUF rx_buf
volatile uint32_t uart_buf_cnt_in = 0;

uint8_t uart_idle_flag = 0;


void printf_init(void) {
    memset((void *)PRINTF_TX_BUF, 0U, USART_TX_DMA_BUF_SIZE);
    // __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
    HAL_UART_Receive_DMA(&huart1, uart_dma_buf, sizeof(uart_dma_buf));
    setvbuf(stdin, NULL, _IONBF, 0);
}


#define PRINTF_TX_FUNC stm_uart_tx
static inline int stm_uart_tx(uint8_t* buf, int len)
{
    return HAL_UART_Transmit_DMA(&huart1, buf, len);
}


void u1_rx_callback(uint8_t data) {
  RETARGET_RX_BUF[uart_buf_cnt_in++%RETARGET_RX_BUF_SIZE] = data;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART1) {
    CLEAR_BIT(huart->Instance->CR2, (USART_CR2_LINEN  | USART_CR2_CLKEN)); 
    CLEAR_BIT(huart->Instance->CR3, (USART_CR3_SCEN   |\
                                     USART_CR3_HDSEL  | USART_CR3_IREN)); 
    huart->ErrorCode  = HAL_UART_ERROR_NONE; 
    huart->gState     = HAL_UART_STATE_READY; 
    huart->RxState    = HAL_UART_STATE_READY; 

    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
    HAL_UART_Receive_DMA(&huart1, uart_dma_buf, sizeof(uart_dma_buf));
  }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART1) {
    u1_rx_callback(uart_dma_buf[1]);
  }

}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART1) {
    u1_rx_callback(uart_dma_buf[0]);
  }
}
