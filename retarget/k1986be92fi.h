// Header: реализация printf scanf для k1946vk035.
// File Name: k1946vk035.h
// Author: Михаил Каа
// Date: 18.04.2025
// Version 0.1 - передача в блокирующем режиме, прием на прерываниях и FIFo

#include <stdio.h>
#include <string.h>
#include <stdint.h>


#define USART_TX_DMA_BUF_SIZE (512U)
uint8_t u0_tx_buf[USART_TX_DMA_BUF_SIZE];
#define PRINTF_TX_BUF u0_tx_buf

#define RETARGET_RX_BUF_SIZE (256U)
static volatile uint8_t rx_buf[RETARGET_RX_BUF_SIZE] = {0};
static volatile uint32_t uart_buf_cnt_in = 0;
#define RETARGET_RX_BUF rx_buf

extern uint32_t SystemCoreClock;
#define RETARGET_UART UART0
#define RETARGET_UART_NUM UART0_Num
#define RETARGET_UART_PORT GPIOB
#define RETARGET_UART_PORT_EN RCU_HCLKCFG_GPIOBEN_Msk
#define RETARGET_UART_PIN_TX_POS 10
#define RETARGET_UART_PIN_RX_POS 11
#define RETARGET_UART_RX_IRQHandler UART0_RX_IRQHandler
#define RETARGET_UART_RX_IRQn UART0_RX_IRQn
#ifndef RETARGET_UART_BAUD
#define RETARGET_UART_BAUD 115200
#endif

void printf_init(void) {
    // TX
    memset((void *)PRINTF_TX_BUF, 0U, USART_TX_DMA_BUF_SIZE);
    
    // RX
    
    
    setvbuf(stdin, NULL, _IONBF, 0);
}


#define PRINTF_TX_FUNC retarget_put_char_test
// Для теста сделаем передачу в блокирующем режиме.
static inline int retarget_put_char_test(uint8_t* buf, int len)
{
    while (len--) {
        // while (RETARGET_UART->FR_bit.BUSY) {};
        // RETARGET_UART->DR = *buf++;
    }
    return 0;
}

// // UART0 Recieve interrupt
// void UART0_RX_IRQHandler(void) {
// // Обработка прерывания приема (RX)
//     if (RETARGET_UART->RIS & UART_RIS_RXRIS_Msk) {
//         // Чтение данных для сброса флага
//         uint8_t data = UART0->DR_bit.DATA;
//         rx_buf[uart_buf_cnt_in++%RETARGET_RX_BUF_SIZE] = data; // Данные в буфер
//         RETARGET_UART->ICR_bit.RXIC = 1; // Очистка прерывания RX
//     }
// }

// // UART0 Transmit interrupt 
// void UART0_TX_IRQHandler(void) {
//     // Обработка TX
//     if (RETARGET_UART->RIS & UART_RIS_TXRIS_Msk) {
//         RETARGET_UART->ICR_bit.TXIC = 1; // Очистка прерывания TX
//     }
// }

// // UART0 Error and Receive Timeout interrupt
// void UART0_E_RT_IRQHandler(void) {

//     // Обработка конца приема пакета (тишина в линии)
//     if (RETARGET_UART->RIS & UART_RIS_RTRIS_Msk) {
//         uint8_t data;
//         while(RETARGET_UART->FR_bit.RXFE == 0) {
//         data = RETARGET_UART->DR_bit.DATA;
//         rx_buf[uart_buf_cnt_in++%RETARGET_RX_BUF_SIZE] = data; // Данные в буфер
//         }
//         RETARGET_UART->ICR_bit.RTIC = 1; // Очистка таймаута приема
//     }

//     // Обработка ошибок (TODO: ...)
//     if (RETARGET_UART->RIS & UART_RIS_FERIS_Msk) UART0->ICR_bit.FEIC = 1; // Framing error
//     if (RETARGET_UART->RIS & UART_RIS_PERIS_Msk) UART0->ICR_bit.PEIC = 1; // Parity error
//     if (RETARGET_UART->RIS & UART_RIS_BERIS_Msk) UART0->ICR_bit.BEIC = 1; // Break error
//     if (RETARGET_UART->RIS & UART_RIS_OERIS_Msk) UART0->ICR_bit.OEIC = 1; // Overrun error
// }

// //   UART0 Transmit Done interrupt
// void UART0_TD_IRQHandler(void) {
//     // Обработка Transmin done
//     if (RETARGET_UART->RIS & UART_RIS_TDRIS_Msk) {
//         RETARGET_UART->ICR_bit.TDIC = 1; // Очистка прерывания TX
//     }
// }
