#if !defined(_WIN32)

#include <putki/sys/files.h>
#include <putki/sys/thread.h>
#include <string.h>
#include <string>
#include <cstdio>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

namespace putki
{
	namespace sys
	{
		char old_path[1024];

		void chdir_push(const char *path)
		{
			getcwd(old_path, 1024);
			chdir(path);
		}

		void chdir_pop()
		{
			chdir(old_path);
		}

		bool stat(const char *path, file_info *out)
		{
			struct ::stat tmp;
			if (!::stat(path, &tmp))
			{
				out->mtime = tmp.st_mtime;
				out->size = tmp.st_size;
				return true;
			}
			return false;
		}

		void search_tree(const char *root, const char *path_from_root, file_enum_t callback, void *userptr)
		{

			DIR *dp = opendir(root);
			if (dp)
			{
				while (dirent *ep = readdir(dp))
				{
					std::string rel_name = std::string(path_from_root) + "/" + ep->d_name;
					std::string full_name = std::string(root) + "/" + ep->d_name;
					if (!strcmp(ep->d_name, ".") || !strcmp(ep->d_name, "..")) {
						continue;
					}

					if (ep->d_type & DT_DIR) {
						search_tree(full_name.c_str(), rel_name.c_str(), callback, userptr);
					}
					else{
						callback(full_name.c_str(), rel_name.substr(1).c_str(), userptr);
					}
				}
				closedir(dp);
			}
		}

		void search_tree(const char *root_directory, file_enum_t callback, void *userptr)
		{
			search_tree(root_directory, "", callback, userptr);
		}

		void mk_dir_for_path(const char *path)
		{
			std::string p(path);

			std::string::size_type i = 0;
			while (true)
			{
				i = p.find_first_of("/", i);
				if (i == std::string::npos) {
					break;
				}

				std::string fp = p.substr(0,i);

				mkdir(fp.c_str(), 0755);
				// std::cout << "Creating [" << fp << "]" << std::endl;
				i++;
			}
		}

		bool write_file(const char *path, const char *str, unsigned long size)
		{
			int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRGRP | S_IWUSR | S_IRUSR | S_IROTH);
			if (fd == -1)
			{
				perror("write_file error");
				return false;
			}
			
			int ret = write(fd, (const char*)str, size);
			if (ret != size)
			{
				std::cerr << "ERROR IS " << ret << std::endl;
				close(fd);
				return false;
			}
			
			close(fd);		
			return true;
		}
	}
}

#endif
