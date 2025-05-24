// Header: реализация printf scanf для k1946vk035.
// File Name: k1946vk035.h
// Author: Михаил Каа
// Date: 18.04.2025
// Version 0.1 - передача в блокирующем режиме, прием на прерываниях и FIFo

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "MDR32FxQI_rst_clk.h"
#include "MDR32FxQI_uart.h"
#include "MDR32FxQI_port.h"


#define USART_TX_DMA_BUF_SIZE (512U)
uint8_t u0_tx_buf[USART_TX_DMA_BUF_SIZE];
#define PRINTF_TX_BUF u0_tx_buf

#define RETARGET_RX_BUF_SIZE (256U)
static volatile uint8_t rx_buf[RETARGET_RX_BUF_SIZE] = {0};
static volatile uint32_t uart_buf_cnt_in = 0;
#define RETARGET_RX_BUF rx_buf

// extern uint32_t SystemCoreClock;
// #define RETARGET_UART UART0
// #define RETARGET_UART_NUM UART0_Num
// #define RETARGET_UART_PORT GPIOB
// #define RETARGET_UART_PORT_EN RCU_HCLKCFG_GPIOBEN_Msk
// #define RETARGET_UART_PIN_TX_POS 10
// #define RETARGET_UART_PIN_RX_POS 11
// #define RETARGET_UART_RX_IRQHandler UART0_RX_IRQHandler
// #define RETARGET_UART_RX_IRQn UART0_RX_IRQn
// #ifndef RETARGET_UART_BAUD
// #endif
#define RETARGET_UART_BAUD 115200

void printf_init(void) {
    // TX
    memset((void *)PRINTF_TX_BUF, 0U, USART_TX_DMA_BUF_SIZE);

    PORT_InitTypeDef PORT_InitStructure;
    UART_InitTypeDef UART_InitStructure;


    RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF, ENABLE);

    PORT_InitStructure.PORT_PULL_UP = PORT_PULL_UP_OFF;
    PORT_InitStructure.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;
    PORT_InitStructure.PORT_PD_SHM = PORT_PD_SHM_OFF;
    PORT_InitStructure.PORT_PD = PORT_PD_DRIVER;
    PORT_InitStructure.PORT_GFEN = PORT_GFEN_OFF;
    PORT_InitStructure.PORT_FUNC = PORT_FUNC_OVERRID;
    PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
    PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;

    // UART_PIN_RX
    PORT_InitStructure.PORT_OE = PORT_OE_IN;
    PORT_InitStructure.PORT_Pin = PORT_Pin_0;
    PORT_Init(MDR_PORTF, &PORT_InitStructure); 

    // UART_PIN_TX
    PORT_InitStructure.PORT_OE = PORT_OE_OUT;
    PORT_InitStructure.PORT_Pin = PORT_Pin_1; 
    PORT_Init(MDR_PORTF, &PORT_InitStructure);

    RST_CLK_PCLKcmd(RST_CLK_PCLK_UART2, ENABLE);

    UART_InitStructure.UART_BaudRate = RETARGET_UART_BAUD; 
    UART_InitStructure.UART_WordLength = UART_WordLength8b;
    UART_InitStructure.UART_StopBits = UART_StopBits1;
    UART_InitStructure.UART_Parity = UART_Parity_Even;
    UART_InitStructure.UART_FIFOMode = UART_FIFO_OFF;
    UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE;

    NVIC_SetPriority(UART2_IRQn, 7);
    NVIC_EnableIRQ(UART2_IRQn);

    // UART_DMAConfig (MDR_UART2, UART_IT_FIFO_LVL_12words, UART_IT_FIFO_LVL_12words);
    // UART_DMACmd(MDR_UART2, UART_DMA_RXE | UART_DMA_ONERR, ENABLE);

    UART_BRGInit(MDR_UART2, UART_HCLKdiv1);
    UART_ITConfig (MDR_UART2, UART_IT_RX, ENABLE);
    UART_Init(MDR_UART2, &UART_InitStructure);
    UART_Cmd(MDR_UART2, ENABLE);
    // RX
    
    
    setvbuf(stdin, NULL, _IONBF, 0);
}


#define PRINTF_TX_FUNC retarget_put_char_test
// Для теста сделаем передачу в блокирующем режиме.
static inline int retarget_put_char_test(uint8_t* buf, int len)
{
    while (len--) {
        UART_SendData(MDR_UART2, *buf++);
        while (MDR_UART2->FR & UART_FR_BUSY) {}
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
