#ifndef UNITS_H
#define UNITS_H

extern char g_prefixUnits[6][2];

typedef enum { UNIT_FIRST, UNIT_PICO, UNIT_NANO, UNIT_MICRO, UNIT_MILI, UNIT_BASE, UNIT_KILO, UNIT_LAST } unit_t;

unit_t Units_CalcIdealUnits(float val);

const char* Units_GetUnitPrefix(unit_t unit);

float Units_GetUnitMultiplier(unit_t unit);

#endif