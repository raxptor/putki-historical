#include <iostream>

#include <putki/pkgloader.h>
#include <putki/pkgmgr.h>
#include <putki/liveupdate/liveupdate.h>

#include <claw/appwindow.h>
#include <claw/render.h>
#include <claw/color.h>
#include <claw/log.h>

#include <game/staticdata.h>

#include <outki/types/claw/ui.h>

#include <cassert>
#include <stdio.h>

// binding up the blob loads.
namespace outki { void bind_claw_loaders(); }

namespace
{
	putki::liveupdate::data *liveupdate;
	outki::globalsettings *settings;
	
}

void frame()
{

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

int main(int argc, char *argv[])
{
	outki::bind_claw_loaders();

/*
	putki::pkgmgr::loaded_package *menu_pkg = putki::pkgloader::from_file("mainmenu.pkg");
	outki::ui_screen *scrn = (outki::ui_screen *) putki::pkgmgr::resolve(menu_pkg, "ui/mainmenu");
*/

	putki::liveupdate::init();
	liveupdate = putki::liveupdate::connect();

	game::load_static_package();

	settings = game::get_global_settings();
	assert(settings);

	claw::appwindow::data *window = claw::appwindow::create(settings->windowtitle, settings->window_width, settings->window_height);
	claw::appwindow::run_loop(window, &frame);

/*	while (claw::appwindow::update(window))
	{
		if (LIVE_UPDATE(&settings))
		{
			claw::appwindow::set_title(window, settings->windowtitle);
		}
		*/
/*
		claw::render::begin(renderer, true, true, claw::rgba2uint(settings->bgcolor));

		LIVE_UPDATE(&scrn);
		for (unsigned int i=0;i<scrn->elements_size;i++)
		{
			LIVE_UPDATE(&scrn->elements[i]);

			outki::ui_element *el = scrn->elements[i];

			if (outki::ui_button *button = scrn->elements[i]->exact_cast<outki::ui_button>())
			{
				
			}
			else if (outki::ui_solidfill *solidfill = scrn->elements[i]->exact_cast<outki::ui_solidfill>())
			{
				claw::render::solid_rect(renderer, el->rect.x, el->rect.y, el->rect.x + el->rect.width, el->rect.y + el->rect.height, claw::rgba2uint(solidfill->fillcolor));
			}
		}

		claw::render::end(renderer);
		claw::render::present(renderer);

		//////////////////////////////////////////////////////

*/		
/*
	claw::render::destroy(renderer);
*/	
	return 0;
}

