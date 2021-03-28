#include "core/init.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main.h"
#include "display/display.h"

void CORE_Init(void) {
	//HAL_GPIO_WritePin(GPIOF, DISP_Pin, GPIO_PIN_SET);

    SCR_DrawRectangle(0, 0, screen.res.w, screen.res.h, true, color_black);
    return;
}
