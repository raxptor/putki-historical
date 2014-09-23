#pragma once

namespace putki
{
	namespace db { struct data; }
	
	struct deferred_loader;

	void load_tree_into_db(const char *sourcepath, db::data *d);
	void load_file_into_db(const char *sourcepath, const char *path, db::data *d, bool resolve);

	// resolve order is resolve0, resolve1
	deferred_loader *create_loader(const char *sourcepath, db::data *resolve0 = 0, db::data *resolve1 = 0);

	void load_file_deferred(deferred_loader *loader, db::data *target, const char *path);
	void loader_incref(deferred_loader *loader);
	void loader_decref(deferred_loader *loader);
}
