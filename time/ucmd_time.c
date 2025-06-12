#include "ucmd.h"
#include <stdio.h>

void time_set(uint8_t h, uint8_t m, uint8_t s);
void time_print(void);

int ucmd_time(int argc, char *argv[]) {
  static uint16_t h = 0, m = 0, s = 0;
  switch (argc) {
  case 1:
    time_print();
    return 0;
    break;
    
  case 4:
    sscanf(&argv[1][0], "%hu", &h);
    sscanf(&argv[2][0], "%hu", &m);
    sscanf(&argv[3][0], "%hu", &s);
    time_set(h, m, s);
    printf("set new time %u:%u:%u\r\n", h, m, s);
    time_print();
    return 0;
    break;

  default:
    return UCMD_CMD_NOT_FOUND;
    break;
  }
}


#ifdef USE_HAL_DRIVER

#include "main.h"

extern RTC_HandleTypeDef hrtc;
RTC_TimeTypeDef time = {0};
RTC_DateTypeDef date = {0};
float time_subsecond = 0.0f;

void time_set(uint8_t h, uint8_t m, uint8_t s) {
  time.Hours = h;
  time.Minutes = m;
  time.Seconds = s;
  HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
}

void time_update(void) {
  HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
  time_subsecond = (((float)(255-time.SubSeconds))*(100.0f/255.0f))/100;
}

void time_print(void) {
  time_update();
  printf("%02d:%02d:%2.3f\r\n", \
    time.Hours, time.Minutes, time.Seconds + time_subsecond);
}

#endif /* USE_HAL_DRIVER */
