#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/resource.h>
#include <putki/builder/pngutil.h>
#include <putki/builder/db.h>

#include <iostream>

#include <inki/types/Texture.h>

struct texbuilder : putki::builder::handler_i
{
	virtual bool handle(putki::builder::data *builder, putki::db::data *input, const char *path, putki::instance_t obj, putki::db::data *output, int obj_phase)
	{
		inki::Texture *texture = (inki::Texture *) obj;
		std::cout << "Processing texture [" << path << "] source <" << texture->Source << ">" << std::endl;

		//
		const char *ptr;
		long long size;
		if (!putki::resource::load(builder, texture->Source.c_str(), &ptr, &size))
		{
			return false;
		}
		else
		{
			// these are the direct load textures.
			inki::TextureOutputPng *pngObj = inki::TextureOutputPng::alloc();
			pngObj->PngPath = std::string("Resources/") + path + ".png";
			pngObj->parent.u0 = 0;
			pngObj->parent.v0 = 0;
			pngObj->parent.u1 = 1.0f;
			pngObj->parent.v1 = 1.0f;

			putki::resource::save_output(builder, pngObj->PngPath.c_str(), ptr, size);
			putki::resource::free(ptr);

			texture->Output = &pngObj->parent;

			std::string path_res(path);
			path_res.append("_out");

			putki::db::insert(output, path_res.c_str(), inki::TextureOutputPng::th(), pngObj);
			return false;
		}

		return false;
	}
};

void register_texture_builder(putki::builder::data *builder)
{
	static texbuilder fb;
	putki::builder::add_data_builder(builder, "Texture", putki::builder::PHASE_FINAL, &fb);
}
