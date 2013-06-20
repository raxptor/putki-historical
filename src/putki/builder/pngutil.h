#include <string>

namespace putki
{
	namespace builder { struct data; }
	namespace pngutil
	{
		std::string write_to_temp(builder::data *builder, const char *path, unsigned int *pixbuf, unsigned int width, unsigned int height);
	}
}
