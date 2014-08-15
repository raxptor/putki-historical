#if !defined(_WIN32)

#include <putki/sys/files.h>
#include <string.h>
#include <string>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

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

		void search_tree(const char *root, const char *path_from_root, file_enum_t callback)
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
						search_tree(full_name.c_str(), rel_name.c_str(), callback);
					}
					else{
						callback(full_name.c_str(), rel_name.substr(1).c_str());
					}
				}
				closedir(dp);
			}
		}

		void search_tree(const char *root_directory, file_enum_t callback)
		{
			search_tree(root_directory, "", callback);
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
	}
}

#endif
