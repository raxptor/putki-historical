#ifndef __CLAW_COLOR_H__
#define __CLAW_COLOR_H__

#include <outki/types/claw/color.h>

namespace claw
{
	inline unsigned int rgba2uint(outki::rgba const & argb) { return (argb.a << 24) | (argb.r << 16) | (argb.g << 8) | (argb.b); }
}

#endif