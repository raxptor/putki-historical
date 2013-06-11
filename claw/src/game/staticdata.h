#ifndef __GAME_STATIC_DATA_H__
#define __GAME_STATIC_DATA_H__

#include <outki/types/game/globalsettings.h>

namespace game
{
	void load_static_package();

	outki::globalsettings *get_global_settings();

}

#endif