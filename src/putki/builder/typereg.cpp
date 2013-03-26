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
        g_reg()->handlers[type] = dt;
    }

	i_type_handler *typereg_get_handler(type_t t)
	{
		return g_reg()->handlers[t];
	}
}