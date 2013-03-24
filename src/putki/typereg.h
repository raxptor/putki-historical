#pragma once

#include <vector.h>

namespace putki
{
    struct field_desc
    {
        std::string name;
        field_type type;
        bool is_array;
    };
    
    typedef void* struct_ptr;
    
    // no idea how to use this.
    struct type_meta
    {
        virtual void field_desc *describe_field(unsigned int i) = 0;
    };
}
