/*
* micros.h
*
*  Created on: Nov 18, 2019
*      Author: Bulanov Konstantin
*
*  Contact information
*  -------------------
*
* e-mail   :  leech001@gmail.com
*/

/*
* |---------------------------------------------------------------------------------
* | Copyright (C) Bulanov Konstantin,2019
* |
* | This program is free software: you can redistribute it and/or modify
* | it under the terms of the GNU General Public License as published by
* | the Free Software Foundation, either version 3 of the License, or
* | any later version.
* |
* | This program is distributed in the hope that it will be useful,
* | but WITHOUT ANY WARRANTY; without even the implied warranty of
* | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* | GNU General Public License for more details.
* |
* | You should have received a copy of the GNU General Public License
* | along with this program.  If not, see <http://www.gnu.org/licenses/>.
* |
* | DWT and delay_us() algorithm used from https://istarik.ru/blog/stm32/131.html and https://github.com/stDstm/Example_STM32F103/tree/master/delay_micros_one_file
* |---------------------------------------------------------------------------------
*/

#ifndef INC_MICROS_H_
#define INC_MICROS_H_


#ifdef USE_HAL_DRIVER
#include "main.h"
__STATIC_INLINE void us_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Разрешаем использовать счётчик.
    DWT->CYCCNT       = 0;
    DWT->CTRL        |= DWT_CTRL_CYCCNTENA_Msk;     // Запускаем счётчик.
}

__STATIC_INLINE void delay_us(uint32_t us) {
    // На первый взгляд работает. Точных замеров не делал, 
    // наверное стоит добавить калибровочные значения.
    uint32_t us_count_tick = (uint32_t)((uint64_t)us * SystemCoreClock / 1000000U);
    uint32_t start_tick = DWT->CYCCNT;
    
    // Учитываем переполнение счётчика:
    while ((DWT->CYCCNT - start_tick) < us_count_tick);
}
 
__STATIC_INLINE uint32_t micros(void) {
    return  DWT->CYCCNT / (SystemCoreClock / 1000000U);
}

// TODO: test me
__STATIC_INLINE uint32_t millis(void) {
    return  DWT->CYCCNT / (SystemCoreClock / 1000000U) / 1000U;
}

#endif /* USE_HAL_DRIVER */

// M Kaa
#ifdef USE_MDR1986VE1T
#include <stdint.h>
#include "MDR32F9Qx_rst_clk.h"
#include "MDR32F9Qx_timer.h"

#define US_TIMER (MDR_TIMER4)
#define US_TIMER_CLK (144*1000*1000)
#define US_RST_CLK_PCLK (RST_CLK_PCLK_TIMER4)

static inline void us_timer_init(void)
{
    TIMER_CntInitTypeDef us_timer = { 0 };
    RST_CLK_PCLKcmd (US_RST_CLK_PCLK, ENABLE);
    TIMER_DeInit(US_TIMER);
    TIMER_BRGInit(US_TIMER, TIMER_HCLKdiv1);
    TIMER_CntStructInit(&us_timer);
    us_timer.TIMER_CounterMode = TIMER_CntMode_ClkFixedDir;
    us_timer.TIMER_Period = 0xffffffff;
    TIMER_CntInit(US_TIMER, &us_timer);
    TIMER_Cmd(US_TIMER, ENABLE);
}

static inline  void delay_us(uint32_t us)
{

    // На первый взгляд работает. Точных замеров не делал, 
    // наверное стоит добавить калибровочные значения.
    uint32_t us_count_tick =  us * ( US_TIMER_CLK / 1000000U);
    // uint32_t us_count_tick = us * (SystemCoreClock / 1000000U);
    uint32_t start_tick = US_TIMER->CNT;
    
    // Учитываем переполнение счётчика:
    while ((US_TIMER->CNT - start_tick) < us_count_tick);
}

// TODO: починить переполнение.
static inline  uint32_t micros(void){
    return US_TIMER->CNT / (US_TIMER_CLK / 1000000U);
}
#endif /* USE_MDR1986VE1T */

#ifdef USE_K1986VE92xI
#include <stdint.h>
#include "MDR32FxQI_rst_clk.h"
#include "MDR32FxQI_timer.h"

#define US_TIMER (MDR_TIMER3)
// #define US_TIMER_CLK (40*1000*1000) // TODO: magic number
#define US_RST_CLK_PCLK (RST_CLK_PCLK_TIMER3)

static inline void ms_timer_init(void)
{
    TIMER_CntInitTypeDef us_timer = { 0 };
    RST_CLK_PCLKcmd (US_RST_CLK_PCLK, ENABLE);
    TIMER_DeInit(US_TIMER);
    TIMER_BRGInit(US_TIMER, TIMER_HCLKdiv32);
    TIMER_CntStructInit(&us_timer);
    us_timer.TIMER_CounterMode = TIMER_CntMode_ClkFixedDir;
    us_timer.TIMER_Prescaler = 1259;
    us_timer.TIMER_Period = 0xffff;
    TIMER_CntInit(US_TIMER, &us_timer);
    TIMER_Cmd(US_TIMER, ENABLE);
}

static inline void delay_ms(uint32_t us)
{

    // На первый взгляд работает. Точных замеров не делал, 
    // наверное стоит добавить калибровочные значения.
    // uint32_t us_count_tick =  us * ( US_TIMER_CLK / 1000000U);
    uint16_t us_count_tick = us;// * 1000U;
    uint16_t start_tick = US_TIMER->CNT;
    
    // Учитываем переполнение счётчика:
    while ((US_TIMER->CNT - start_tick) < us_count_tick);
}

// TODO: починить переполнение.
static inline  uint32_t millis(void) {
    return US_TIMER->CNT;
}
#endif /* USE_MDR32F9Q2I */


// static inline void delay_ms(uint32_t ms)
// {
//     delay_us(ms*1000);
// }

#endif /* INC_MICROS_H_ */
