#include "core/init.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "display/display.h"

void CORE_Init(void) {
    SCR_DrawRectangle(0, 0, screen.res.w, screen.res.h, true, color_black);
    SCR_DrawRectangle(100, 100, 50, 50, true, color_red);

    return;
}
