//
// Created by jack on 3/25/2025.
//

#ifndef PYTHONCONTEXT_H
#define PYTHONCONTEXT_H
#include "PrecompHeader.h"

#include <pocketpy.h>



class StdoutCapturingVM : public pkpy::VM
{
public:
    std::ostringstream mstdout;

    void stdout_write(const pkpy::Str& s) override {
        mstdout << std::string(s.sv());
    }
};

class PythonContext {
public:
    StdoutCapturingVM* vm = new StdoutCapturingVM();
    PythonContext()
    {

    }
    ~PythonContext()
    {
        delete vm;
    }

};



#endif //PYTHONCONTEXT_H
