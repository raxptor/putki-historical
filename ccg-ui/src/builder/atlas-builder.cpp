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

#include <inki/types/ccg-ui/Texture.h>
#include <inki/types/ccg-ui/Atlas.h>

#include <builder/binpacker/maxrects_binpack.h> // NOTE: Claw include.

#include "textureconfig.h"

namespace
{
	#define KERNEL_SIZE 5
	struct sample_kernel
	{
		float k[KERNEL_SIZE * KERNEL_SIZE];
	};

	void make_sample_kernel(sample_kernel *out, float rt, float suppression = 0.90f, float adjx=0, float adjy=0)
	{
		const float d = sqrt(-2 * logf(suppression)) / (rt * 2.0f * 3.1415f);
		const float mul = 1.0f / sqrtf(2.0f * 3.1415f * d);

		float sum = 0;
		for (int x=0;x<KERNEL_SIZE;x++)
		{
			for (int y=0;y<KERNEL_SIZE;y++)
			{
				const float cx = x - KERNEL_SIZE / 2 - adjx;
				const float cy = y - KERNEL_SIZE / 2 - adjy;
				const float val = mul * expf( - float(cx*cx + cy*cy) / (2.0f * d *d));
				out->k[y*KERNEL_SIZE+x] = val;
				sum += val;
			}
		}

		for (int x=0;x<KERNEL_SIZE*KERNEL_SIZE;x++)
			out->k[x] /= sum;
	}

	// point sampling.
	unsigned long sample(const unsigned int * px, int s_width, int s_height, int t_width, int t_height, const sample_kernel &kernel, int x, int y)
	{
		float _x = float(x) * float(s_width) / float(t_width);
		float _y = float(y) * float(s_height) / float(t_height);

		sample_kernel blah;
		make_sample_kernel(&blah, float(t_width) / float(s_width), 0.05f, _x - floorf(_x), _y - floorf(_y));

		unsigned int outpx = 0;
		for (int component=0;component<32;component+=8)
		{
			int cx = int(_x) - KERNEL_SIZE / 2;
			int cy = int(_y) - KERNEL_SIZE / 2;
			
			unsigned char *pxc = (unsigned char*) px;
			pxc += component / 8;

			float sum = 0;
			for (int y=0;y<KERNEL_SIZE;y++)
			{
				for (int x=0;x<KERNEL_SIZE;x++)
				{
					int px = cx + x;
					int py = cy + y;
					if (px < 0) px = 0;
					if (py < 0) py = 0;
					if (px > s_width - 1)  px = s_width  - 1;
					if (py > s_height - 1) py = s_height - 1;
					sum += float(pxc[4 * (py * s_width + px)]) * blah.k[y*KERNEL_SIZE+x];
				}
			}

			if (sum > 255) sum = 255;
			if (sum < 0) sum = 255;
			int val = (int) sum;
			outpx |= (val << component);
		}
	
		return outpx;

		//return px[s_width * y + x];
	}
}

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
			ir.width = png.width;
			ir.height = png.height;
			ir.id = loaded.size();
			inputRects.push_back(ir);

			if (ir.width > max_width) max_width = ir.width;
			if (ir.height > max_height) max_height = ir.height;

			loaded.push_back(png);
			std::cout << "      " << atlas->Inputs[i] << " loaded [" << png.width << "x" << png.height << "]" << std::endl;
		}
		

		for (int i=0;i<g_outputTexConfigs;i++)
		{
			int out_width = 1;
			int out_height = 1;

			const TextureScaleConf &scaleConfig = g_outputTexConf[i];

			const int m_w = (int)ceilf(max_width * scaleConfig.scale);
			const int m_h = (int)ceilf(max_height * scaleConfig.scale);

			// start values that can actually contain the items.
			while (out_width  < m_w) out_width *= 2;
			while (out_height < m_h) out_height *= 2;

			std::vector<rbp::Rect> packedRects;

			// pack until we know how to do it!
			while (true)
			{
				rbp::MaxRectsBinPack pack(out_width, out_height);

				std::vector< rbp::InputRect > tmpCopy = inputRects;
				for (unsigned int i=0;i<tmpCopy.size();i++)
				{
					tmpCopy[i].width  = (int)floorf(0.5f + tmpCopy[i].width * scaleConfig.scale) + 2 * border;
					tmpCopy[i].height = (int)floorf(0.5f + tmpCopy[i].height * scaleConfig.scale) + 2 * border;
				}

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
					outBmp[y*out_width+x] = (x^y) & 1 ? 0xff101010 : 0xfff0f0f0;
				}
			}

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

	
				sample_kernel krn;
				make_sample_kernel(&krn, scaleConfig.scale);
				
				for (int y=0;y<out.height;y++)
				{
					 for (int x=0;x<out.width;x++)
						outBmp[out_width * (out.y + y) + (out.x + x)] = sample(g.pixels, g.width, g.height, out.width - border * 2, out.height - border * 2, krn, x - border, y - border);
				}

				inki::AtlasEntry e;
				e.id = putki::db::pathof_including_unresolved(input, atlas->Inputs[packedRects[k].id]);
				e.u0 = float(out.x + border) / float(out_width);
				e.v0 = float(out.y + border) / float(out_height);
				e.u1 = float(out.x + out.width - border) / float(out_width);
				e.v1 = float(out.y + out.height - border) / float(out_height);

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
