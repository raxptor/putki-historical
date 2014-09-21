#pragma once

namespace putki
{
	namespace db { struct data; }
	
	struct deferred_loader;

	void load_tree_into_db(const char *sourcepath, db::data *d);
	void load_file_into_db(const char *sourcepath, const char *path, db::data *d, bool resolve, db::data *resolve_db = 0);
	
	deferred_loader *create_loader(const char *sourcepath);
	void load_file_deferred(deferred_loader *loader, db::data *target, const char *path, db::data *resolve_db = 0);
	void loader_clear_resolve_dbs(deferred_loader *loader);
	void loader_incref(deferred_loader *loader);
	void loader_decref(deferred_loader *loader);
}
