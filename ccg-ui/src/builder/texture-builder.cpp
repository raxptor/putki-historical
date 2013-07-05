#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/resource.h>
#include <putki/builder/pngutil.h>
#include <putki/builder/build-db.h>
#include <putki/builder/db.h>

#include <iostream>

#include <inki/types/Texture.h>

#include "textureconfig.h"

struct texbuilder : putki::builder::handler_i
{
	virtual bool handle(putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj, putki::db::data *output, int obj_phase)
	{
		inki::Texture *texture = (inki::Texture *) obj;
		
		// this is used for atlas lookups later.
		texture->id = path;

		if (texture->NoOutput)
		{
			// This will not build the TextureOutput, but the asset desc itself will exist.
			return false;
		}

		std::cout << "Processing texture [" << path << "] source <" << texture->Source << ">" << std::endl;

		putki::pngutil::loaded_png png;
		if (putki::pngutil::load(putki::resource::real_path(builder, texture->Source.c_str()).c_str(), &png))
		{
			// these are the direct load textures.
			inki::TextureOutputPng *pngObj = inki::TextureOutputPng::alloc();
			pngObj->PngPath = std::string("Resources/") + path + ".png";
			pngObj->parent.u0 = 0;
			pngObj->parent.v0 = 0;
			pngObj->parent.u1 = 1.0f;
			pngObj->parent.v1 = 1.0f;

			putki::pngutil::write_to_output(builder, pngObj->PngPath.c_str(), png.pixels, png.width, png.height);
			putki::pngutil::free(&png);

			texture->Output = &pngObj->parent;

			// add.
			std::string path_res(path);
			path_res.append("_out");
			putki::db::insert(output, path_res.c_str(), inki::TextureOutputPng::th(), pngObj);

			putki::build_db::add_output(record, path_res.c_str());
			return false;
		}

		return false;
	}
};

void register_texture_builder(putki::builder::data *builder)
{
	static texbuilder fb;
	putki::builder::add_data_builder(builder, "Texture", putki::builder::PHASE_INDIVIDUAL, &fb);
}
