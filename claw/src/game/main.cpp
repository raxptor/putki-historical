#include <iostream>

#include <putki/pkgloader.h>
#include <putki/pkgmgr.h>
#include <putki/liveupdate/liveupdate.h>

#include <claw/appwindow.h>
#include <claw/render.h>
#include <claw/color.h>

#include <game/staticdata.h>

#include <cassert>
#include <stdio.h>

// binding up the blob loads.
namespace outki { void bind_claw_loaders(); }

int main(int argc, char *argv[])
{
	outki::bind_claw_loaders();

	putki::liveupdate::init();
	putki::liveupdate::data *liveupdate = putki::liveupdate::connect();

	game::load_static_package();

	outki::globalsettings *settings = game::get_global_settings();
	assert(settings);

	claw::appwindow::data *window = claw::appwindow::create(settings->windowtitle, settings->window_width, settings->window_height);
	claw::render::data *renderer = claw::render::create(window);

	while (claw::appwindow::update(window))
	{
		LIVE_UPDATE(&settings);

		claw::render::begin(renderer, true, true, claw::rgba2uint(settings->bgcolor));
		claw::render::end(renderer);
		claw::render::present(renderer);

		//////////////////////////////////////////////////////

		if (liveupdate)
		{
			if (!putki::liveupdate::connected(liveupdate))
			{
				putki::liveupdate::disconnect(liveupdate);
				liveupdate = 0;
			}
			else
			{
				putki::liveupdate::update(liveupdate);
			}
		}
	}

	claw::render::destroy(renderer);
	
	return 0;
}

