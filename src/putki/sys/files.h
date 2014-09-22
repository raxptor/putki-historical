#ifndef __PUTKI_SYS_FILES__
#define __PUTKI_SYS_FILES__

namespace putki
{
	namespace sys
	{
		struct file_info
		{
			long long mtime;
			long long size;
		};
		
		typedef void (*file_enum_t) (const char *fullname, const char *name, void *userptr);
		
		bool stat(const char *path, file_info *out);
		void search_tree(const char *root_directory, file_enum_t callback, void *userptr);
		void mk_dir_for_path(const char *path);
		bool write_file(const char *path, const char *str, unsigned long size);

		void chdir_push(const char *path);
		void chdir_pop();
	}

}

#endif
