#if defined(_WIN32)

#include <putki/sys/files.h>

#include <windows.h>
#include <direct.h>
#include <sys/stat.h>
#include <vector>
#include <string>
#include <iostream>

namespace putki
{

	namespace sys
	{
		void chdir_push(const char *path)
		{
			SetCurrentDirectory(path);
		}

		void chdir_pop()
		{
		}

		void search_tree_internal(const char *root_directory, file_enum_t callback, int cut_length, void *userptr)
		{
			std::string root(root_directory);
			if (root.empty()) {
				root = ".";
			}

			std::string files_in_path = root + "/*.*";

			std::vector<std::string> dirs;

			WIN32_FIND_DATAA fd;
			HANDLE f = FindFirstFileA(files_in_path.c_str(), &fd);
			while (f != INVALID_HANDLE_VALUE)
			{
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..")) {
						dirs.push_back(root + "/" + fd.cFileName);
					}
				}
				else
				{
					std::string nice_name(root + "/" + fd.cFileName);
					std::string full_name = nice_name;

					nice_name = nice_name.erase(0, cut_length);

					callback(full_name.c_str(), nice_name.c_str(), userptr);
				}

				if (!FindNextFileA(f, &fd)) {
					break;
				}
			}

			FindClose(f);

			while (!dirs.empty())
			{
				search_tree_internal(dirs.back().c_str(), callback, cut_length, userptr);
				dirs.pop_back();
			}
		}

		void search_tree(const char *root_directory, file_enum_t callback, void *userptr)
		{
			search_tree_internal(root_directory, callback, strlen(root_directory) + 1, userptr);
		}

		bool write_file(const char *path, const char *str, unsigned long size)
		{
			HANDLE hFile;
			DWORD wmWritten;
			hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			WriteFile(hFile, str, (DWORD)(size), &wmWritten, NULL);
			CloseHandle(hFile);
			return wmWritten == size;
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

				_mkdir(fp.c_str());
				i++;
			}
		}
	}
}

#endif

