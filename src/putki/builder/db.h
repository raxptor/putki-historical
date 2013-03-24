#pragma once

namespace putki
{
	struct db;

	void db_insert(db *d, const char *path, type_t t, type_inst *i);	
}
