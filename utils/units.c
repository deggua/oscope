#include "utils/units.h"

#include <math.h>
#include <stddef.h>

const char g_prefixUnits[7][2] = {"p", "n", "u", "m", "", "k"};

unit_t Units_CalcIdealUnits(float val) {
    val         = fabs(val);
    unit_t unit = UNIT_BASE;

    if (val < 1.0f) {
        while (val < 1.0f) {
            val *= 1000.0f;
            unit--;
            if (unit == UNIT_FIRST) {
                return UNIT_PICO;
            }
        }
        return unit;
    } else if (val >= 1000.0f) {
        return UNIT_KILO;
    } else {
        return UNIT_BASE;
    }
}

const char* Units_GetUnitPrefix(unit_t unit) {
    if (unit <= UNIT_FIRST || unit >= UNIT_LAST) {
        return NULL;
    }

    return g_prefixUnits[unit - 1];
}

float Units_GetUnitMultiplier(unit_t unit) {
    if (unit <= UNIT_FIRST || unit >= UNIT_LAST) {
        return NAN;
    }

    float mults[7] = {10.0e-12f, 10.0e-9f, 10.0e-6f, 10.0e-3f, 1.0f, 10.0e3f};
    return mults[unit - 1];
}