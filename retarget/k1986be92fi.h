// Header: реализация printf scanf для К1986ВЕ92FI (маркировка MDR1211FI).
// File Name: k1986be92fi.h
// Author: Михаил Каа
// Date: 25.05.2025
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

#define RETARGET_UART_BAUD 115200

void printf_init(void) {
    memset((void *)PRINTF_TX_BUF, 0U, USART_TX_DMA_BUF_SIZE);

    PORT_InitTypeDef PORT_InitStructure = {0};
    UART_InitTypeDef UART_InitStructure = {0};

    // Тактирование GPIO.
    RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF, ENABLE);

    PORT_InitStructure.PORT_PULL_UP     = PORT_PULL_UP_OFF;
    PORT_InitStructure.PORT_PULL_DOWN   = PORT_PULL_DOWN_OFF;
    PORT_InitStructure.PORT_PD_SHM      = PORT_PD_SHM_OFF;
    PORT_InitStructure.PORT_PD          = PORT_PD_DRIVER;
    PORT_InitStructure.PORT_GFEN        = PORT_GFEN_OFF;
    PORT_InitStructure.PORT_FUNC        = PORT_FUNC_OVERRID;
    PORT_InitStructure.PORT_SPEED       = PORT_SPEED_MAXFAST;
    PORT_InitStructure.PORT_MODE        = PORT_MODE_DIGITAL;

    // UART_PIN_RX
    PORT_InitStructure.PORT_OE          = PORT_OE_IN;
    PORT_InitStructure.PORT_Pin         = PORT_Pin_0;
    PORT_Init(MDR_PORTF, &PORT_InitStructure); 

    // UART_PIN_TX
    PORT_InitStructure.PORT_OE          = PORT_OE_OUT;
    PORT_InitStructure.PORT_Pin         = PORT_Pin_1; 
    PORT_Init(MDR_PORTF, &PORT_InitStructure);

    RST_CLK_PCLKcmd(RST_CLK_PCLK_UART2, ENABLE);

    UART_InitStructure.UART_BaudRate    = RETARGET_UART_BAUD; 
    UART_InitStructure.UART_WordLength  = UART_WordLength8b;
    UART_InitStructure.UART_StopBits    = UART_StopBits1;
    UART_InitStructure.UART_Parity      = UART_Parity_No;
    UART_InitStructure.UART_FIFOMode    = UART_FIFO_ON;
    UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE;

    NVIC_SetPriority(UART2_IRQn, 7);
    NVIC_EnableIRQ(UART2_IRQn);

    UART_BRGInit(MDR_UART2, UART_HCLKdiv1);
    UART_ITConfig (MDR_UART2, UART_IT_RX | UART_IT_RT, ENABLE);
    UART_Init(MDR_UART2, &UART_InitStructure);
    UART_Cmd(MDR_UART2, ENABLE);
    
    setvbuf(stdin, NULL, _IONBF, 0);
}


#define PRINTF_TX_FUNC retarget_put_char_test
// Для теста сделаем передачу в блокирующем режиме.
static inline int retarget_put_char_test(uint8_t* buf, int len)
{
    while (len--) {
        while (MDR_UART2->FR & UART_FR_BUSY) {}
        UART_SendData(MDR_UART2, *buf++);
    }
    return 0;
}


void UART2_IRQHandler( void )
{
    if( UART_GetITStatus(MDR_UART2, UART_IT_TX ) == SET )
    {
        UART_ClearITPendingBit(MDR_UART2, UART_IT_TX);
    }

    if( UART_GetITStatus(MDR_UART2, UART_IT_RX ) == SET )
    {
        while ((MDR_UART2->FR & UART_FR_RXFE) == RESET)
        {
            rx_buf[uart_buf_cnt_in++%RETARGET_RX_BUF_SIZE] = UART_ReceiveData(MDR_UART2); // Данные в буфер
        }
        
        UART_ClearITPendingBit(MDR_UART2, UART_IT_RX);
    }

    if( UART_GetITStatus(MDR_UART2, UART_IT_RT ) == SET )
    {
        while ((MDR_UART2->FR & UART_FR_RXFE) == RESET)
        {
            rx_buf[uart_buf_cnt_in++%RETARGET_RX_BUF_SIZE] = UART_ReceiveData(MDR_UART2); // Данные в буфер
        }
        
        UART_ClearITPendingBit(MDR_UART2, UART_IT_RT);
    }
}

