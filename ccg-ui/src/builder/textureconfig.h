#pragma once

namespace
{
	struct TextureScaleConf
	{
		float scale;
	};

	const int g_outputTexConfigs = 1;
	static const TextureScaleConf g_outputTexConf[g_outputTexConfigs] =
	{
		{1.0f}
	};
}
