
#include <string>

namespace putki
{
	namespace builder { struct data; }

	namespace resource
	{
		void free(const char *data);
		bool load(builder::data *builder, const char *path, const char **outBytes, long long *outSize);

		std::string signature(builder::data *builder, const char *path);
		std::string save_temp(builder::data *builder, const char *path, const char *bytes, long long length);
		std::string save_output(builder::data *builder, const char *path, const char *bytes, long long length);

		// translate path to something that can be used to open a file with fopen
		std::string real_path(builder::data *builder, const char *path);
	}
}
