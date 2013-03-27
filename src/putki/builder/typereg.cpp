#include "typereg.h"
#include <map>
#include <string>
#include <iostream>

namespace putki
{
    struct registry
    {
        std::map<std::string, type_handler_i*> handlers;
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
    
    void typereg_register(const char *type, type_handler_i *dt)
    {
        g_reg()->handlers[type] = dt;
    }

	type_handler_i *typereg_get_handler(type_t t)
	{
		return g_reg()->handlers[t];
	}
}