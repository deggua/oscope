#include <stdint.h>

#include "utils/geometry.h"

typedef struct {
    // member variables
    char* _text;

    // member functions
    void (* _Destructor)(gui_label_t*);

    gui_ret_t (* SetPosition)(gui_label_t*, const int32_t, const int32_t);
    gui_ret_t (* SetText)(gui_label_t*, const char*);
} gui_label_t;