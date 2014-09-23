#include "resource.h"

#include <putki/builder/builder.h>
#include <putki/sys/files.h>
#include <putki/sys/thread.h>

#include <fstream>
#include <iostream>
#include <string>

extern "C" {
	#include <md5/md5.h>
}

namespace putki
{
	namespace resource
	{
		sys::mutex _mtx;
		
		std::string real_path(builder::data *builder, const char *path)
		{
			std::string full_path;
			const char *ps = path;

			if (path[0] == '%')
			{
				full_path = builder::tmp_path(builder);
				ps++;
			}
			else
			{
				full_path = builder::res_path(builder);
			}

			return full_path + "/" + ps;
		}

		bool load(builder::data *bld, const char *path, const char **outBytes, long long *outSize)
		{
			sys::scoped_maybe_lock lk(&_mtx);
			
			std::string full_path = real_path(bld, path);

			std::ifstream f(full_path.c_str(), std::ios::binary);
			if (!f.good())
			{
				APP_WARNING("Failed to load resource [" << full_path << "]")
				return false;
			}

			f.seekg(0, std::ios::end);
			std::streampos size = f.tellg();
			f.seekg(0, std::ios::beg);

			char *b = new char[(size_t)size];
			f.read(b, size);

			*outBytes = b;
			*outSize = (long long) size;
			return true;
		}

		void free(const char *data)
		{
			delete [] data;
		}

		std::string signature(builder::data *bld, const char *path)
		{
			// optimize this.
			const char *bytes;
			long long sz;
			if (!load(bld, path, &bytes, &sz))
			{
				return "(missing)";
			}
			else
			{
				char signature[64];
				char signature_string[64];

				md5_buffer(bytes, (long)sz, signature);
				md5_sig_to_string(signature, signature_string, 64);

				delete [] bytes;
				return signature_string;
			}
		}

		std::string save_temp(builder::data *builder, const char *path, const char *bytes, long long length)
		{
			std::string out_path = std::string(builder::tmp_path(builder)) + "/" + path;
			sys::mk_dir_for_path(out_path.c_str());

			std::ofstream f(out_path.c_str(), std::ios::binary);
			f.write(bytes, length);
			return std::string("%") + path;
		}

		std::string save_output(builder::data *builder, const char *path, const char *bytes, long long length)
		{
			std::string out_path = std::string(builder::out_path(builder)) + "/" + path;
			sys::mk_dir_for_path(out_path.c_str());

			std::ofstream f(out_path.c_str(), std::ios::binary);
			f.write(bytes, length);
			return path;
		}
	}
}
