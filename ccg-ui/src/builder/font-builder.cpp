#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/resource.h>
#include <putki/builder/pngutil.h>
#include <putki/builder/db.h>

#include <iostream>

#include <inki/types/Font.h>
#include <inki/types/Texture.h>

#include <ft2build.h>
#include FT_FREETYPE_H


struct fontbuilder : putki::builder::handler_i
{
	virtual bool handle(putki::builder::data *builder, putki::db::data *input, const char *path, putki::instance_t obj, putki::db::data *output, int obj_phase)
	{
		inki::Font *font = (inki::Font *) obj;

		std::cout << "Building font [" << path << "] with source [" << font->Source << "]" << std::endl;

		const char *fnt_data;
		long long fnt_len;
		if (putki::resource::load(builder, font->Source.c_str(), &fnt_data, &fnt_len))
		{
			std::cout << "Loaded file with size " << fnt_len << std::endl;

			FT_Library ft;
			if (FT_Init_FreeType(&ft))
			{
				putki::builder::build_error(builder, "Could not initialize freetype");
				return false;
			}

			FT_Face face;
			if (FT_New_Memory_Face(ft, (const FT_Byte *)fnt_data, (FT_Long)fnt_len, 0, &face))
			{
				putki::builder::build_error(builder, "Could not load font face");
				return false;
			}

			if (FT_Set_Pixel_Sizes(face, 0, 20))
			{
				putki::builder::build_error(builder, "Could not set char or pixel size.");
				return false;
			}

			const char *str = "ABC";

			unsigned int *blah = new unsigned int[64*128];
			for (int y=0;y<128;y++)
				for (int i=0;i<64;i++)
					blah[y*64+i] = 0x00;

			for (int i=0;i<(int)strlen(str);i++)
			{
				int idx = FT_Get_Char_Index(face, (int)str[i]);
				if (FT_Load_Glyph(face, idx, FT_LOAD_NO_BITMAP))
				{
					putki::builder::build_error(builder, "Could not load glyph face.");
					return false;
				}

				if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
				{
					putki::builder::build_error(builder, "Could not render glyph.");
					continue;
				}

				FT_Bitmap *bmp = &face->glyph->bitmap;
				for (int y=0;y<bmp->rows;y++)
				{
					for (int x=0;x<bmp->width;x++)
					{
						blah[(32 * i + y) * 64 + x] = bmp->buffer[y * bmp->width + x] * 0x01000000 | 0xffffff;
					}
				}

				std::cout << "Glyph is at index " << idx << std::endl;
			}
				
			std::string output_atlas_path = std::string(path) + "_atlas.png";
			output_atlas_path = putki::pngutil::write_to_temp(builder, output_atlas_path.c_str(), blah, 64, 128);

			std::cout << "Font has " << face->num_glyphs << " glyphs." << std::endl;
			std::cout << "Wrote atlas to [" << output_atlas_path << "]" << std::endl;

			// create & insert texture.
			{
				putki::type_handler_i *tex = inki::get_Texture_type_handler();
				inki::Texture *texture = (inki::Texture *) tex->alloc();
				texture->sourcefile = output_atlas_path;
				putki::db::insert(output, (std::string(path) + "_atlas").c_str(), tex, texture);

				// give font the texture.
				font->OutputTexture = texture;
			}
		}
		else
		{
			putki::builder::build_error(builder, "Load failed.");
		}
		
		return false;
	}
};

void register_font_builder(putki::builder::data *builder)
{
	static fontbuilder fb;
	putki::builder::add_data_builder(builder, "Font", putki::builder::PHASE_1, &fb);
}

