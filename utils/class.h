#ifndef CLASS_H
#define CLASS_H

typedef struct {
    void (* _Destructor)(void*);
} class_t;

void (* Class_GetDestructor(void))(void*);
void Class_New(class_t* this);

#endif
