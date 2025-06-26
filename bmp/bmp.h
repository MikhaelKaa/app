
// Утилита для работы с файлами изображений в формате bmp.
// M Kaa
// 26.06.2025

#ifndef _BMP_TOOL_
#define _BMP_TOOL_

// BMP image tool.
int ucmd_bmp(int argc, char *argv[]);

// Screen function & parameters
extern uint16_t BMP_scr_width;
extern uint16_t BMP_scr_heigth;
extern void (*BMP_draw_pixel)(uint16_t x, uint16_t y, uint16_t pixel);

#endif // _BMP_TOOL_

