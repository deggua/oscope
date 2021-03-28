#ifndef USER_CONTROLS_H
#define USER_CONTROLS_H

#include <stdbool.h>

#include "utils/scope.h"

typedef enum { JOY_DIR_NONE, JOY_DIR_UP, JOY_DIR_DOWN, JOY_DIR_LEFT, JOY_DIR_RIGHT } joy_dir_t;

typedef enum {
	ROT_DIR_NONE, ROT_DIR_CW, ROT_DIR_CCW
} rot_dir_t;

typedef struct {
    joy_dir_t dir;
    bool      pressed;
} joy_state_t;

typedef struct {
	rot_dir_t dir;
	bool pressed;
} rot_state_t;

joy_state_t DRV_Joy_GetState(void);
rot_state_t DRV_Rot_GetState(scope_channel_t channel);

#endif
