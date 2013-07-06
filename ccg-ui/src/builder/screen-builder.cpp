#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/resource.h>
#include <putki/builder/build-db.h>
#include <putki/builder/db.h>

#include <inki/types/Screen.h>

#include "textureconfig.h"

struct screenbuilder : putki::builder::handler_i
{
	virtual bool handle(putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj, putki::db::data *output, int obj_phase)
	{
		inki::UIScreen *screen = (inki::UIScreen *) obj;

		if (screen->SnapScale)
		{
			for (int i=0;i<g_outputTexConfigs;i++)
				screen->ScalingForSnapping.push_back(g_outputTexConf[i].scale);
		}

		return false;
	}
};

void register_screen_builder(putki::builder::data *builder)
{
	static screenbuilder fb;
	putki::builder::add_data_builder(builder, "UIScreen", putki::builder::PHASE_INDIVIDUAL, &fb);
}

