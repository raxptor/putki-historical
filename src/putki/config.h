#ifndef __PUTKI_CONFIG_H__
#define __PUTKI_CONFIG_H__

namespace putki
{
	namespace cfg
	{
		struct data;

		data* load(const char *name);
		data* merge(data *first, data *second);

		void free(data *d);

		int get_int(data *d, const char *section, const char *key, int def);
		float get_float(data *d, const char *section, const char *key, float def);
		const char* get_string(data *d, const char *section, const char *key, const char *def);
	}
};

#endif