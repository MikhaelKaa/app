#ifndef _UCMD_TIME_HAL_
#define _UCMD_TIME_HAL_

#include "main.h"

void time_update(void);

extern RTC_TimeTypeDef time;
extern RTC_DateTypeDef date;

#endif /* _UCMD_TIME_HAL_ */

