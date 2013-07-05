#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/resource.h>
#include <putki/builder/pngutil.h>
#include <putki/builder/build-db.h>
#include <putki/builder/db.h>

#include <iostream>
#include <vector>
#include <sstream>

#include <inki/types/Texture.h>
#include <inki/types/Atlas.h>

#include <builder/binpacker/maxrects_binpack.h> // NOTE: Claw include.

#include "textureconfig.h"

struct atlasbuilder : putki::builder::handler_i
{
	virtual bool handle(putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj, putki::db::data *output, int obj_phase)
	{
		inki::Atlas *atlas = (inki::Atlas *) obj;
		std::cout << "Processing atlas [" << path << "]" << std::endl;

		std::vector<putki::pngutil::loaded_png> loaded;
		std::vector<rbp::InputRect> inputRects;

		static unsigned int fakepixel = 0xffff00ff;

		int max_width = 1;
		int max_height = 1;

		int border = 2;

		for (unsigned int i=0;i<atlas->Inputs.size();i++)
		{
			putki::pngutil::loaded_png png;
			if (!putki::pngutil::load(putki::resource::real_path(builder, atlas->Inputs[i]->Source.c_str()).c_str(), &png))
			{
				putki::builder::build_error(builder, "Failed to load png");
				png.pixels = &fakepixel;
				png.width = 1;
				png.height = 1;
			}

			rbp::InputRect ir;
			ir.width = png.width + 2 * border;
			ir.height = png.height + 2 * border;
			ir.id = loaded.size();
			inputRects.push_back(ir);

			if (ir.width > max_width) max_width = ir.width;
			if (ir.height > max_height) max_height = ir.height;

			loaded.push_back(png);
			std::cout << "      " << atlas->Inputs[i] << " loaded [" << png.width << "x" << png.height << "]" << std::endl;
		}
		

		int out_width = 1;
		int out_height = 1;

		// start values that can actually contain the items.
		while (out_width  < max_width) out_width *= 2;
		while (out_height < max_height) out_height *= 2;

		std::vector<rbp::Rect> packedRects;

		// pack until we know how to do it!
		while (true)
		{
			rbp::MaxRectsBinPack pack(out_width, out_height);

			std::vector< rbp::InputRect > tmpCopy = inputRects;
			pack.Insert(tmpCopy, packedRects, rbp::MaxRectsBinPack::RectBottomLeftRule);

			if (packedRects.size() == inputRects.size())
			{
				break;
			}
			else
			{
				if (out_height > out_width)
					out_width *= 2;
				else
					out_height *= 2;

				packedRects.clear();
			}
		}

		// make the atlas.
		unsigned int * outBmp = new unsigned int[out_width * out_height];
		for (int y=0;y<out_height;y++)
		{
			for (int x=0;x<out_width;x++)
			{
				outBmp[y*out_width+x] = (x^y) & 1 ? 0xff101010 : 0xff303030;
			}
		}

		for (int i=0;i<g_outputTexConfigs;i++)
		{
			inki::AtlasOutput ao;
			
			ao.Width = out_width;
			ao.Height = out_height;
			ao.Scale = g_outputTexConf[i].scale;

			for (unsigned int k=0;k<packedRects.size();k++)
			{
				putki::pngutil::loaded_png const &g = loaded[packedRects[k].id];
				rbp::Rect const &out = packedRects[k];

				int blah = 0;
				if (g_outputTexConf[i].scale != 1)
					blah = rand();
				
				for (unsigned int y=0;y<out.height;y++)
				{
					int rl_y = (int)y - border;

					if (rl_y < 0) rl_y = 0;
					if (rl_y >= g.height) rl_y = g.height-1;

					for (unsigned int x=0;x<out.width;x++)
					{
						int rl_x = (int)x - border;
						if (rl_x < 0) rl_x = 0;
						if (rl_x >= g.width) rl_x = g.width-1;
						outBmp[out_width * (out.y + y) + (out.x + x)] = g.pixels[g.width * rl_y + rl_x] | blah;
					}
				}

				inki::AtlasEntry e;
				e.id = putki::db::pathof_including_unresolved(input, atlas->Inputs[packedRects[k].id]);
				e.u0 = float(out.x) / float(out_width);
				e.v0 = float(out.y) / float(out_height);
				e.u1 = float(out.x + g.width) / float(out_width);
				e.v1 = float(out.y + g.height) / float(out_height);

				ao.Entries.push_back(e);
			}

			std::stringstream str;
			str << path << "_atlas_" << i;

			std::string output_atlas_path = str.str() + "_atlas.png";
			output_atlas_path = putki::pngutil::write_to_temp(builder, output_atlas_path.c_str(), outBmp, out_width, out_height);

			{
				std::string outpath = str.str();

				// create new texture.
				inki::Texture *texture = inki::Texture::alloc();
				texture->Source = output_atlas_path;
				putki::db::insert(output, outpath.c_str(), inki::Texture::th(), texture);
				putki::build_db::add_output(record, outpath.c_str());

				ao.Texture = texture;
				atlas->Outputs.push_back(ao);
			}
		}

		return false;
	}
};

void register_atlas_builder(putki::builder::data *builder)
{
	static atlasbuilder fb;
	putki::builder::add_data_builder(builder, "Atlas", putki::builder::PHASE_INDIVIDUAL, &fb);
}
