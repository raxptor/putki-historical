#include <putki/builder-config.h>
#include <putki/config.h>
#include <string>

namespace putki
{

	namespace data_builder_cfg
	{
		namespace
		{
			std::string data_input_path;
			std::string data_output_path;

		};
		const char *input_path()
		{
			return data_input_path.c_str();
		}

		const char *output_path()
		{
			return data_output_path.c_str();
		}

		void load(const char *config_file)
		{
			cfg::data *d = cfg::load(config_file);

			// extract paths.
			data_input_path  = cfg::get_string(d, "paths.data", "input", "./");
			data_output_path = cfg::get_string(d, "paths.data", "output", "./");

			cfg::free(d);
		}
	}

}
