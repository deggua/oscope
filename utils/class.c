#include "utils/class.h"

static void Destructor(class_t* this) {
    return;
}

void Class_New(class_t* this) {
    this->_Destructor = &Destructor;
    return;
}

void (* Class_GetDestructor(void))(class_t*) {
    return &Destructor;
}

void Delete(class_t* class) {
    class->_Destructor(class);
    return;
}