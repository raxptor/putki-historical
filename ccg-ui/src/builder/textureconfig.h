#pragma once

namespace
{
	struct TextureScaleConf
	{
		float scale;
	};

	const int g_outputTexConfigs = 4;
	static const TextureScaleConf g_outputTexConf[g_outputTexConfigs] =
	{
		{1.0f},
		{0.75f},
		{0.50f},
		{0.10f}
	};
}
