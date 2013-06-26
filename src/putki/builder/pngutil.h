#include <string>

namespace putki
{
	namespace builder { struct data; }
	namespace pngutil
	{
		std::string write_to_temp(builder::data *builder, const char *path, unsigned int *pixbuf, unsigned int width, unsigned int height);
		std::string write_to_output(builder::data *builder, const char *path, unsigned int *pixbuf, unsigned int width, unsigned int height);

		struct loaded_png
		{
			unsigned int *pixels;
			unsigned int width, height;
			unsigned int bpp; // always 32 for now
		};

		bool load(const char *path, loaded_png *out);
		void free(loaded_png *png);
	}
}
