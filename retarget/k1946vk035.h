// Header: реализация printf scanf для k1946vk035.
// File Name: k1946vk035.h
// Author: Михаил Каа
// Date: 18.04.2025
// Version 0.1 - передача в блокирующем режиме, прием на прерываниях и FIFo

#include <stdio.h>
#include "K1921VK035.h"
#include <string.h>

#define RETARGET_TX_BUF_SIZE (512U)
uint8_t u0_tx_buf[RETARGET_TX_BUF_SIZE];
#define RETARGET_TX_BUF u0_tx_buf

#define RETARGET_RX_BUF_SIZE (256U)
static volatile uint8_t rx_buf[RETARGET_RX_BUF_SIZE] = {0};
static volatile uint32_t uart_buf_cnt_in = 0;
// static volatile uint32_t rx_buf_cnt = 0;
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
    memset((void *)RETARGET_TX_BUF, 0U, RETARGET_TX_BUF_SIZE);
    SystemCoreClockUpdate();
    uint32_t baud_icoef = SystemCoreClock / (16 * RETARGET_UART_BAUD);
    uint32_t baud_fcoef = ((SystemCoreClock / (16.0f * RETARGET_UART_BAUD) - baud_icoef) * 64 + 0.5f);
    uint32_t uartclk_ref = RCU_UARTCFG_UARTCFG_CLKSEL_OSICLK;

    RCU->HCLKCFG |= RETARGET_UART_PORT_EN;
    RCU->HRSTCFG |= RETARGET_UART_PORT_EN;
    RETARGET_UART_PORT->ALTFUNCSET = (1 << RETARGET_UART_PIN_TX_POS) | (1 << RETARGET_UART_PIN_RX_POS);
    RETARGET_UART_PORT->DENSET = (1 << RETARGET_UART_PIN_TX_POS) | (1 << RETARGET_UART_PIN_RX_POS);

    if (RCU->SYSCLKSTAT_bit.SYSSTAT == RCU_SYSCLKCFG_SYSSEL_OSECLK)
        uartclk_ref = RCU_UARTCFG_UARTCFG_CLKSEL_OSECLK;
    else if (RCU->SYSCLKSTAT_bit.SYSSTAT == RCU_SYSCLKCFG_SYSSEL_OSICLK)
        uartclk_ref = RCU_UARTCFG_UARTCFG_CLKSEL_OSICLK;
    else if (RCU->SYSCLKSTAT_bit.SYSSTAT == RCU_SYSCLKCFG_SYSSEL_PLLCLK)
        uartclk_ref = RCU_UARTCFG_UARTCFG_CLKSEL_PLLCLK;
    else if (RCU->SYSCLKSTAT_bit.SYSSTAT == RCU_SYSCLKCFG_SYSSEL_PLLDIVCLK)
        uartclk_ref = RCU_UARTCFG_UARTCFG_CLKSEL_PLLDIVCLK;
    RCU->UARTCFG[RETARGET_UART_NUM].UARTCFG = (uartclk_ref << RCU_UARTCFG_UARTCFG_CLKSEL_Pos) |
                                              RCU_UARTCFG_UARTCFG_CLKEN_Msk |
                                              RCU_UARTCFG_UARTCFG_RSTDIS_Msk;
    RETARGET_UART->IBRD = baud_icoef;
    RETARGET_UART->FBRD = baud_fcoef;
    RETARGET_UART->LCRH = UART_LCRH_FEN_Msk | (3 << UART_LCRH_WLEN_Pos);
    RETARGET_UART->CR = UART_CR_TXE_Msk | UART_CR_RXE_Msk | UART_CR_UARTEN_Msk;

    // RX
    RETARGET_UART->IMSC = \
    UART_IMSC_TXIM_Msk  | UART_IMSC_RXIM_Msk | \
    UART_IMSC_RTIM_Msk  | UART_IMSC_TDIM_Msk ;

    NVIC_EnableIRQ(UART0_RX_IRQn);
    NVIC_EnableIRQ(UART0_E_RT_IRQn);
    
    
    setvbuf(stdin, NULL, _IONBF, 0);
}


#define RETARGET_TX_FUNC retarget_put_char_test
// Для теста сделаем передачу в блокирующем режиме.
static inline int retarget_put_char_test(uint8_t* buf, int len)
{
    while (len--){
        while (RETARGET_UART->FR_bit.BUSY) {};
        RETARGET_UART->DR = *buf++;
    }
    return 0;
}

// UART0 Recieve interrupt
void UART0_RX_IRQHandler(void) {
// Обработка прерывания приема (RX)
    if (RETARGET_UART->RIS & UART_RIS_RXRIS_Msk) {
        // Чтение данных для сброса флага
        uint8_t data = UART0->DR_bit.DATA;
        rx_buf[uart_buf_cnt_in++%RETARGET_RX_BUF_SIZE] = data; // Данные в буфер
        RETARGET_UART->ICR_bit.RXIC = 1; // Очистка прерывания RX
    }
}

// UART0 Transmit interrupt 
void UART0_TX_IRQHandler(void) {
    // Обработка TX
    if (RETARGET_UART->RIS & UART_RIS_TXRIS_Msk) {
        RETARGET_UART->ICR_bit.TXIC = 1; // Очистка прерывания TX
    }
}

// UART0 Error and Receive Timeout interrupt
void UART0_E_RT_IRQHandler(void) {

    // Обработка конца приема пакета (тишина в линии)
    if (RETARGET_UART->RIS & UART_RIS_RTRIS_Msk) {
        uint8_t data;
        while(RETARGET_UART->FR_bit.RXFE == 0) {
        data = RETARGET_UART->DR_bit.DATA;
        rx_buf[uart_buf_cnt_in++%RETARGET_RX_BUF_SIZE] = data; // Данные в буфер
        }
        RETARGET_UART->ICR_bit.RTIC = 1; // Очистка таймаута приема
    }

    // Обработка ошибок (TODO: ...)
    if (RETARGET_UART->RIS & UART_RIS_FERIS_Msk) UART0->ICR_bit.FEIC = 1; // Framing error
    if (RETARGET_UART->RIS & UART_RIS_PERIS_Msk) UART0->ICR_bit.PEIC = 1; // Parity error
    if (RETARGET_UART->RIS & UART_RIS_BERIS_Msk) UART0->ICR_bit.BEIC = 1; // Break error
    if (RETARGET_UART->RIS & UART_RIS_OERIS_Msk) UART0->ICR_bit.OEIC = 1; // Overrun error
}

//   UART0 Transmit Done interrupt
void UART0_TD_IRQHandler(void) {
    // Обработка Transmin done
    if (RETARGET_UART->RIS & UART_RIS_TDRIS_Msk) {
        RETARGET_UART->ICR_bit.TDIC = 1; // Очистка прерывания TX
    }
}
