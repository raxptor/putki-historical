#pragma once

namespace putki
{
	namespace db { struct data; }

	void load_tree_into_db(const char *sourcepath, db::data *d);
	void load_file_into_db(const char *sourcepath, const char *path, db::data *d, bool resolve, db::data *resolve_db = 0);
}
