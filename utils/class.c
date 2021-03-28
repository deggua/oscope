#include "utils/class.h"

static void Destructor(void* this) {
    return;
}

void Class_New(class_t* this) {
    this->_Destructor = &Destructor;
    return;
}

void (* Class_GetDestructor(void))(void*) {
    return &Destructor;
}

void Delete(class_t* class) {
    class->_Destructor(class);
    return;
}
