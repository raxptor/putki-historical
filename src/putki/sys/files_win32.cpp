#if defined(_WIN32)

#include <putki/sys/files.h>
#include <windows.h>
#include <vector>
#include <string>

namespace putki
{

	namespace sys
	{
		void search_tree_internal(const char *root_directory, file_enum_t callback, int cut_length)
		{
			std::string root(root_directory);
			if (root.empty())
				root = ".";

			std::string files_in_path = root + "\\*.*";

			std::vector<std::string> dirs;

			WIN32_FIND_DATAA fd;
			HANDLE f = FindFirstFileA(files_in_path.c_str(), &fd);
			while (f != INVALID_HANDLE_VALUE)
			{
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, ".."))
						dirs.push_back(root + "\\" + fd.cFileName);
				}
				else
				{
					std::string nice_name(root + "\\" + fd.cFileName);
					std::string full_name = nice_name;

					nice_name = nice_name.erase(0, cut_length);

					for (std::string::size_type i=0;i!=nice_name.size();i++)
						if (nice_name[i] == '\\')
							nice_name[i] = '/';

					callback(full_name.c_str(), nice_name.c_str());
				}

				if (!FindNextFileA(f, &fd))
					break;
			}

			FindClose(f);

			while (!dirs.empty())
			{
				search_tree_internal(dirs.back().c_str(), callback, cut_length);
				dirs.pop_back();
			}
		}

		void search_tree(const char *root_directory, file_enum_t callback)
		{
			search_tree_internal(root_directory, callback, strlen(root_directory) + 1);
		}

	}
}

#endif

