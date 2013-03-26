#if !defined(_WIN32)

#include <putki/sys/files.h>
#include <string.h>
#include <string>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

namespace putki
{
    namespace sys
    {
		void search_tree(const char *root_directory, file_enum_t callback)
        {
            callback("src/types/core.typedef", "core.typedef");
        }
        
        void mk_dir_for_path(const char *path)
        {
            std::string p(path);
            
            std::string::size_type i = 0;
            while (true)
            {
                i = p.find_first_of("/", i);
                if (i == std::string::npos)
                    break;
                
                std::string fp = p.substr(0,i);
                
                mkdir(fp.c_str(), 0755);
                std::cout << "Creating [" << fp << "]" << std::endl;
                i++;
            }
        }
    }
}

#endif