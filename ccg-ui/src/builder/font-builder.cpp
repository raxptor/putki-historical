#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/resource.h>
#include <putki/builder/pngutil.h>
#include <putki/builder/db.h>

#include <iostream>

#include <inki/types/Font.h>

struct fontbuilder : putki::builder::handler_i
{
	virtual bool handle(putki::builder::data *builder, putki::db::data *input, const char *path, putki::db::data *output, int obj_phase)
	{
		putki::type_handler_i *th;
		putki::instance_t obj;
		if (putki::db::fetch(input, path, &th, &obj))
		{
			inki::Font *font = (inki::Font *) obj;

			std::cout << "Building font [" << path << "] with source [" << font->Source << "]" << std::endl;

			const char *fnt_data;
			long long fnt_len;
			if (putki::resource::load(builder, font->Source.c_str(), &fnt_data, &fnt_len))
			{
				std::cout << "Loaded file with size " << fnt_len << std::endl;
			}
			else
			{
				std::cout << "Load failed" << std::endl;
			}

			const char *gurka = "Gurka!";
			std::string spath = putki::resource::save_temp(builder, "kalas/skitkorv", gurka, strlen(gurka));

			unsigned int *blah = new unsigned int[64*128];
			for (int y=0;y<128;y++)
				for (int i=0;i<64;i++)
					blah[y*64+i] = i * 0x010101 | 0xff000000;

			putki::pngutil::write_to_temp(builder, "kalas/bajskorv.png", blah, 64, 128);

			std::cout << "Wrote tmp to <" << spath << ">!" << std::endl;

			return false;
		}
		return false;
	}
};

void register_font_builder(putki::builder::data *builder)
{
	static fontbuilder fb;
	putki::builder::add_data_builder(builder, "Font", putki::builder::PHASE_1, &fb);
}

