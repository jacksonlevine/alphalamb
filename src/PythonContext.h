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
        std::string wrapped_code =
            "try:\n"
            "    exec(\"\"\"" + code + "\"\"\")\n"
            "except Exception as e:\n"
            "    print('[ERROR]', type(e).__name__ + ':', e)";

        py_exec(wrapped_code.c_str(), "Puter", EXEC_MODE, NULL);
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

        // Redirect sys.stderr
        exec("import sys\n"
             "class CustomStderr:\n"
             "    def write(self, msg):\n"
             "        print(msg, end='')\n"
             "sys.stderr = CustomStderr()");
        exec("import sys\n"
         "def custom_excepthook(exc_type, exc_value, traceback):\n"
         "    print(f'Error: {exc_value}', end='')\n"
         "sys.excepthook = custom_excepthook");
    }

};



#endif //PYTHONCONTEXT_H
