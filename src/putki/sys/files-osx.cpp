#include <putki/sys/files.h>

namespace putki
{
    namespace sys
    {
		void search_tree(const char *root_directory, file_enum_t callback)
        {
            callback("blahonga", "kalaspuff");
        }
    }
}