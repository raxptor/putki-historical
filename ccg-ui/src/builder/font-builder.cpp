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

#include <builder/binpacker/maxrects_binpack.h> // NOTE: Claw include.

struct fontbuilder : putki::builder::handler_i
{
	virtual bool handle(putki::builder::data *builder, putki::buildrecord::data *record, putki::db::data *input, const char *path, putki::instance_t obj, putki::db::data *output, int obj_phase)
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

			if (FT_Set_Pixel_Sizes(face, 0, 30))
			{
				putki::builder::build_error(builder, "Could not set char or pixel size.");
				return false;
			}

			const char *str = "AbcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";


			int numGlyphs = strlen(str);

			std::vector< rbp::InputRect > packs;

			struct GlyphInfo
			{
				int w, h;
				char *data;
			};

			std::vector< GlyphInfo > glyphs;

			for (int i=0;i<numGlyphs;i++)
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

				GlyphInfo g;
				g.data = new char[bmp->width * bmp->rows];
				g.w = bmp->width;
				g.h = bmp->rows;
				glyphs.push_back(g);

				for (int y=0;y<bmp->rows;y++)
				{
					for (int x=0;x<bmp->width;x++)
					{
						g.data[y * bmp->width + x] = bmp->buffer[y * bmp->width + x];
					}
				}

				rbp::InputRect next;
				next.id = i;
				next.width = bmp->width;
				next.height = bmp->rows;
				packs.push_back(next);
			}

			std::vector<rbp::Rect> packedRects;


			int out_width = 16;
			int out_height = 16;

			while (true)
			{
				packedRects.clear();

				rbp::MaxRectsBinPack pack(out_width, out_height);
				std::vector< rbp::InputRect > tmpCopy = packs;
				pack.Insert(tmpCopy, packedRects, rbp::MaxRectsBinPack::RectBottomLeftRule);

				if (packedRects.size() == packs.size())
				{
					break;
				}
				else
				{
					if (out_height > out_width)
						out_width *= 2;
					else
						out_height *= 2;
				}
			}

			unsigned int * outBmp = new unsigned int[out_width * out_height];
			for (int y=0;y<out_height;y++)
			{
				for (int x=0;x	<out_width;x++)
				{
					outBmp[y*out_width+x] = (x^y) & 1 ? 0xff101010 : 0xff303030;
				}
			}


			for (unsigned int k=0;k<packedRects.size();k++)
			{
				GlyphInfo const &g = glyphs[packedRects[k].id];
				rbp::Rect const &out = packedRects[k];
				//std::cout << "Packed rect[" << k << "] is at " << packedRects[k].x << "/" << packedRects[k].y << "  id:" << packedRects[k].id << std::endl;

				for (int y=0;y<g.h;y++)
				{
					for (int x=0;x<g.w;x++)
					{
						outBmp[out_width * (out.y + y) + (out.x + x)] = g.data[g.w * y + x] * 0x010101 | 0xff000000;
					}
				}
			}

			std::string output_atlas_path = std::string(path) + "_atlas.png";
			output_atlas_path = putki::pngutil::write_to_temp(builder, output_atlas_path.c_str(), outBmp, out_width, out_height);

			std::cout << "Font has " << face->num_glyphs << " glyphs." << std::endl;
			//std::cout << "Wrote atlas to [" << output_atlas_path << "]" << std::endl;

			// create & insert texture.
			
			{
				std::string outpath = (std::string(path) + "_atlas");

				// create new texture.
				inki::Texture *texture = inki::Texture::alloc();
				texture->Source = output_atlas_path;
				putki::db::insert(output, outpath.c_str(), inki::Texture::th(), texture);

				// give font the texture.
				font->OutputTexture = texture;

				// add it so it will be built.
				putki::buildrecord::add_output(record, outpath.c_str());
			}
			
			return false;
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
	putki::builder::add_data_builder(builder, "Font", putki::builder::PHASE_INDIVIDUAL, &fb);
}

