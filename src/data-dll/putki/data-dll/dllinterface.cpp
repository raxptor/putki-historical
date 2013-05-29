#include "dllinterface.h"
#include "dllinternal.h"

#include <putki/builder/typereg.h>

void app_bind_putki_types();
void app_bind_putki_types_dll();

namespace putki
{
	struct data_dll : public data_dll_i
	{
		mem_instance* create_instance(const char *type)
		{
			type_handler_i *th = putki::typereg_get_handler(type);

			mem_instance *ni = new mem_instance();
			ni->th = th;

			return 0;
		}

		mem_instance* disk_load(const char *path) 
		{
			return 0;
		}

		ext_type_handler_i* type_by_index(unsigned int i)
		{
			return get_ext_type_handler_by_index(i);
		}
	};

	data_dll_i* __cdecl load_data_dll()
	{
		// bind at startup.
		app_bind_putki_types();
		app_bind_putki_types_dll();

		static data_dll dl;
		return &dl;
	}

}