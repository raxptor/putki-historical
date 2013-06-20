
#include <string>

namespace putki
{
	namespace builder { struct data; }

	namespace resource
	{
		void free(const char *data);

		bool load(builder::data *builder, const char *path, const char **outBytes, long long *outSize);
		std::string save_temp(builder::data *builder, const char *path, const char *bytes, long long length);
	}
}
