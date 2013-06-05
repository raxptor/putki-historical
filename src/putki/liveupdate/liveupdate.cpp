#include <putki/builder/db.h>
#include <putki/liveupdate/liveupdate.h>

namespace putki
{

	namespace liveupdate
	{
#if defined(_WIN32)

		struct data
		{

		};
		
		data* start_server(const char *name)
		{
			return new data();
		}

		void stop_server(data *which)
		{
			delete which;
		}

		void send_update(data *lu, db::data *data, const char *path)
		{

		}

#else
		// null implementation
		struct data { };
		data* start_server(const char *name) { return new data(); }
		void stop_server(data *which) { delete which; }
		void send_update(data *lu, db::data *data, const char *path) { }
#endif
	}

}
