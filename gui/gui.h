#ifndef GUI_H
#define GUI_H

typedef enum {
    GUI_RET_SUCCESS,               // the operation succeeded
    GUI_RET_FAILURE_NOMEM,         // the operation failed, out of memory
    GUI_RET_FAILURE_INVALID,       // the operation failed, invalid parameters
    GUI_RET_FAILURE_NULLPTR,       // the operation failed, nullptr used
    GUI_RET_FAILURE_MISSING,       // the operation failed, the target could not be found
    GUI_RET_FAILURE_UNINITIALIZED, // the operation failed, the target was unintiailized
    GUI_RET_ERROR                  // the operation encountered an unexpected error
} gui_ret_t;

#endif