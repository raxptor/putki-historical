#ifndef __PUTKI_BUILDER_CONF_H__
#define __PUTKI_BUILDER_CONF_H__

namespace putki
{
	namespace data_builder_cfg
	{
		const char *input_path();
		const char *output_path();

		void load(const char *config_file);
	}

	namespace code_builder_cfg
	{
		const char *input_path();
		const char *output_path();
	}
}

#endif
