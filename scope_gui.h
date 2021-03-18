#include <stdint.h>

#include "scope_display.h"
#include "scope_config.h"

#define GUI_CHILDREN_INIT_ALLOC 1UL

#define GUI_POSX_MIN 0
#define GUI_POSX_MAX 1024
#define GUI_POSY_MIN 0
#define GUI_POSY_MAX 1024

#define GUI_BUTTON_WIDTH_MIN 1
#define GUI_BUTTON_WIDTH_MAX 128
#define GUI_BUTTON_HEIGHT_MIN 1
#define GUI_BUTTON_HEIGHT_MAX 128

/* GUI element types */
typedef enum {
    GUI_ELEMENT_BUTTON, 
    GUI_ELEMENT_WINDOW, 
    GUI_ELEMENT_LABEL, 
    GUI_ELEMENT_GRAPH
} gui_element_t;

typedef enum {
    GUI_RET_SUCCESS,            //the operation succeeded
    GUI_RET_FAILURE_NOMEM,      //the operation failed, out of memory
    GUI_RET_FAILURE_INVALID,    //the operation failed, invalid parameters
    GUI_RET_FAILURE_NULLPTR,    //the operation failed, nullptr used
    GUI_RET_ERROR               //the operation encountered an unexpected error
} gui_ret_t;

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
