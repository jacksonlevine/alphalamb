//
// Created by jack on 3/25/2025.
//

#ifndef PYTHONCONTEXT_H
#define PYTHONCONTEXT_H

#include "PrecompHeader.h"


class PythonContext {
    PythonContext()
    {
        static void* _ = &py_initialize();
    }
};



#endif //PYTHONCONTEXT_H
