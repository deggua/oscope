#include <stdint.h>

#include "scope_display.h"
#include "scope_config.h"

#define GUI_CHILDREN_INIT_ALLOC 1UL

/* GUI element types */
typedef enum {
    GUI_BUTTON, GUI_WINDOW, GUI_LABEL, GUI_GRAPH
} gui_type_t;

/* GUI element structs */
typedef struct {
    point_t pos;
    rect_t dim;
    char* _text;
} gui_button_t;

typedef struct {
    point_t pos;
    char* _text;
} gui_label_t;


typedef struct {
    point_t pos;
    rect_t dim;

    float xlower, xupper;
    float ylower, yupper;

    graph_waveform_t* _waveforms[GUI_MAX_WAVEFORMS];
} gui_graph_t;

typedef struct {
    point_t pos;
    rect_t dim;
} gui_window_t;

typedef struct GUI_OBJECT {
    void* _elem;

    struct GUI_OBJECT* _parent;
    struct GUI_OBJECT** _children;
    size_t _num_children_cur;
    size_t _num_children_max;

    gui_type_t _type;
    void (* _destructor)(void*);

    bool visible;
} gui_object_t;
