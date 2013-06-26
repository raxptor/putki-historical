#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/resource.h>
#include <putki/builder/pngutil.h>
#include <putki/builder/db.h>

#include <iostream>
#include <vector>
#include <sstream>

#include <inki/types/Texture.h>
#include <inki/types/Atlas.h>

struct atlasbuilder : putki::builder::handler_i
{
	virtual bool handle(putki::builder::data *builder, putki::buildrecord::data *record, putki::db::data *input, const char *path, putki::instance_t obj, putki::db::data *output, int obj_phase)
	{
		inki::Atlas *atlas = (inki::Atlas *) obj;
		std::cout << "Processing atlas [" << path << "]" << std::endl;

		/*
		putki::instance_t oa;
		putki::type_handler_i *th;
		if (putki::db::fetch(input, path, &th, &oa))
		{
			find_refs fr;
			fr.atlas = (inki::Atlas *) oa;
			putki::db::read_all(input, &fr);

			for (int scale=0;scale<(1 + atlas->ScaleDowns);scale++)
			{
				std::stringstream ss;
				ss << path << "_" << scale;

				inki::AtlasOutput res;

				for (unsigned int i=0;i<fr.sources.size();i++)
				{
					inki::AtlasEntry ae;
					ae.u0 = 0;
					ae.v0 = 0;
					ae.u1 = 1;
					ae.v1 = 1;
					ae.id = "gurka";
					res.entries.push_back(ae);
				}

				atlas->outputs.push_back(res);
			}
		}
		*/

		return false;
	}
};

void register_atlas_builder(putki::builder::data *builder)
{
	static atlasbuilder fb;
	putki::builder::add_data_builder(builder, "Atlas", putki::builder::PHASE_GLOBAL, &fb);
}
