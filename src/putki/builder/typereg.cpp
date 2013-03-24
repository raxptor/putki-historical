#include "typereg.h"
#include <map>
#include <string>
#include <iostream>

namespace putki
{
    struct registry
    {
        std::map<std::string, i_type_handler*> handlers;
    };
    
    namespace
    {
        registry *g_reg()
        {
            static registry r;
            return &r;
        }
    }
    
    void typereg_init()
    {

    }
    
    void typereg_register(const char *type, i_type_handler *dt)
    {
        std::cout << "Registering type [" << type << "]" << std::endl;
        g_reg()->handlers[type] = dt;
    }
}