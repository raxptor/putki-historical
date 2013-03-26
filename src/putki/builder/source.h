#pragma once

namespace putki
{
	namespace db { struct data; }

	void init_db_with_source(const char *sourcepath, db::data *d);
}
