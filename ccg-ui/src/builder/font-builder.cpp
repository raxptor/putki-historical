#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/resource.h>
#include <putki/builder/pngutil.h>
#include <putki/builder/build-db.h>
#include <putki/builder/db.h>

#include <iostream>
#include <sstream>

#include <inki/types/Font.h>
#include <inki/types/Texture.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <builder/binpacker/maxrects_binpack.h> // NOTE: Claw include.

struct fontbuilder : putki::builder::handler_i
{
	virtual bool handle(putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj, putki::db::data *output, int obj_phase)
	{
		inki::Font *font = (inki::Font *) obj;

		std::cout << "Building font [" << path << "] with source [" << font->Source << "]" << std::endl;

		for (int i='a';i<'z';i++)
			font->Characters.push_back(i);
		for (int i='A';i<'Z';i++)
			font->Characters.push_back(i);


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

			for (unsigned int sz=0;sz<font->PixelSizes.size();sz++)
			{
				if (FT_Set_Pixel_Sizes(face, 0, font->PixelSizes[sz]))
				{
					putki::builder::build_error(builder, "Could not set char or pixel size.");
					return false;
				}

				std::vector< rbp::InputRect > packs;

				struct TmpGlyphInfo
				{
					int w, h;
					int bearingX, bearingY;
					int advance;
					char *data;
				};

				std::vector<TmpGlyphInfo> glyphs;

				inki::FontOutput up;
				up.PixelSize = font->PixelSizes[sz];

				up.BBoxMinY = 1000000;
				up.BBoxMaxY = -100000;

				int border = 1;

				for (unsigned int i=0;i<font->Characters.size();i++)
				{
					int idx = FT_Get_Char_Index(face, font->Characters[i]);
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

					TmpGlyphInfo g;
					g.data = new char[bmp->width * bmp->rows];
					g.w = bmp->width;
					g.h = bmp->rows;
					g.bearingX = face->glyph->metrics.horiBearingX;
					g.bearingY = face->glyph->metrics.horiBearingY;
					g.advance = face->glyph->metrics.horiAdvance;
					glyphs.push_back(g);

					const int y0 = g.bearingY - 64 * g.h;
					const int y1 = g.bearingY;
					if (y0 < up.BBoxMinY) up.BBoxMinY = y0;
					if (y1 > up.BBoxMaxY) up.BBoxMaxY = y1;

					for (unsigned int j=0;j<font->Characters.size();j++)
					{
						FT_Vector kerning;
						if (!FT_Get_Kerning(face, font->Characters[i], font->Characters[j], FT_KERNING_DEFAULT, &kerning))
						{
							if (kerning.x != 0)
							{
								up.KerningCharL.push_back(font->Characters[i]);
								up.KerningCharR.push_back(font->Characters[j]);
								up.KerningOfs.push_back(kerning.x);
							}
						}
					}

					for (int y=0;y<bmp->rows;y++)
					{
						for (int x=0;x<bmp->width;x++)
						{
							g.data[y * bmp->width + x] = bmp->buffer[y * bmp->width + x];
						}
					}

					rbp::InputRect next;
					next.id = i;
					next.width = bmp->width + 2 * border;
					next.height = bmp->rows + 2 * border;
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
						outBmp[y*out_width+x] = (x^y) & 1 ? 0xff101010 : 0xff808080;
					}
				}

				for (unsigned int k=0;k<packedRects.size();k++)
				{
					TmpGlyphInfo const &g = glyphs[packedRects[k].id];
					rbp::Rect const &out = packedRects[k];

					// insert into output.
					inki::FontGlyph fg;
					fg.glyph = font->Characters[packedRects[k].id];
					fg.u0 = float(out.x + border) / float(out_width);
					fg.v0 = float(out.y + border) / float(out_height);
					fg.u1 = float(out.x + border + g.w) / float(out_width);
					fg.v1 = float(out.y + border + g.h) / float(out_height);
					fg.pixelWidth = g.w;
					fg.pixelHeight = g.h;
					fg.bearingX = g.bearingX;
					fg.bearingY = - g.bearingY;
					fg.advance = g.advance;

					up.Glyphs.push_back(fg);

					for (int y=0;y<g.h;y++)
					{
						for (int x=0;x<g.w;x++)
						{
							outBmp[out_width * (out.y + y + border) + (out.x + x + border)] = g.data[g.w * y + x] * 0x01000000 | 0xffffff;
						}
					}
				}

				std::stringstream ss;
				ss << path << "_" << font->PixelSizes[sz] << "_glyphs";

				std::string outpath = ss.str();
				std::string output_atlas_path = ss.str() + ".png";
				output_atlas_path = putki::pngutil::write_to_temp(builder, output_atlas_path.c_str(), outBmp, out_width, out_height);

				// create & insert texture.
				{
					// create new texture.
					inki::Texture *texture = inki::Texture::alloc();
					texture->Source = output_atlas_path;
					putki::db::insert(output, outpath.c_str(), inki::Texture::th(), texture);

					// give font the texture.
					up.OutputTexture = texture;
					font->Outputs.push_back(up);

					// add it so it will be built.
					putki::build_db::add_output(record, outpath.c_str());
				}
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

