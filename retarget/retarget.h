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

#endif /* RETARGET */
