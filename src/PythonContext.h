//
// Created by jack on 3/25/2025.
//

#ifndef PYTHONCONTEXT_H
#define PYTHONCONTEXT_H
#include "PrecompHeader.h"

extern "C" {
#include "pocketpy.h"
}

class PythonContext
{
public:
    static std::ostringstream g_output;


    void exec(const std::string& code)
    {
        py_exec(code.c_str(), "Puter", EXEC_MODE, NULL);
    }

    static bool custom_print(int argc, py_Ref argv) {
        for (int i = 0; i < argc; i++) {
            if (i > 0) g_output << " ";

            py_Ref arg = py_arg(i);

            py_str(arg);
            const char* str = py_tostr(py_retval());

            g_output << str;
            py_pop();
        }

        g_output << std::endl;

        return true;
    }


    PythonContext()
    {
        py_initialize();

        py_Ref r0 = py_getreg(0);
        py_newnativefunc(r0, custom_print);
        py_setglobal(py_name("print"), r0);

    }

};



#endif //PYTHONCONTEXT_H
