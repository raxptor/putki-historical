#ifndef __PUTKI_SYS_FILES__
#define __PUTKI_SYS_FILES__

namespace putki
{
	namespace sys
	{
		typedef void (*file_enum_t) (const char *fullname, const char *name);

		void search_tree(const char *root_directory, file_enum_t callback);
	}

}

#endif
