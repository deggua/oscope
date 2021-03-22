#ifndef GUI_RENDERER_H
#define GUI_RENDERER_H

#define GUI_NUM_ACCENTS 4

typedef struct {
    color_t foreground, background;
    color_t border;
    color_t text;
    color_t accents[GUI_NUM_ACCENTS];
} gui_theme_t;

#endif