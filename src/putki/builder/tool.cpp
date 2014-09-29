#include <putki/builder/db.h>
#include <putki/builder/tool.h>
#include <putki/builder/log.h>

namespace putki
{
	namespace
	{
		struct dep_checker : public depwalker_i
		{	
			db::data *primary;
			db::data *secondary;
			bool follow_aux_ptrs;
			bool follow_ptrs;
			int check_flags;
			
			bool pointer_pre(putki::instance_t *on, const char *ptr_type)
			{
				if (!*on) return false;
				
				const char *path_unres = db::is_unresolved_pointer(primary, *on);
				const char *path_real  = db::pathof(primary, *on);
			
				if (secondary)
				{
					if (!path_unres) path_unres = db::is_unresolved_pointer(secondary, *on);
					if (!path_real) path_real = db::pathof(secondary, *on);
				}

				if (path_unres && path_real)
					APP_ERROR("verify_obj: Encountered both unresolved and pathy pointer: " << *on);
				
				if ((check_flags & REQUIRE_RESOLVED) && path_unres)
					APP_ERROR("verify_obj: Has pointer " << *on << " with unresolved path [" << path_unres << "]")
				if ((check_flags & REQUIRE_HAS_PATHS) && !path_real)
					APP_ERROR("verify_obj: Has pointer " << *on << " without path!")
				
				/*
				if (path_real)
					APP_DEBUG("verify_spam ptr " << *on << " path:" << path_real)
				if (path_unres)
					APP_DEBUG("verify_spam ptr " << *on << " unres:" << path_unres)
				*/
					
				if (!follow_aux_ptrs && !path_real)
					APP_WARNING("verify_obj: Not follow aux, but cannot figure out path!")
				else if (!follow_aux_ptrs && db::is_aux_path(path_real))
					return false;
					
				return follow_ptrs;
			}

			void pointer_post(putki::instance_t *on)
			{
			
			}
		};
	}

	void verify_obj(db::data *primary, db::data *secondary, type_handler_i *th, instance_t obj, int check_flags, bool follow_aux_ptrs, bool follow_ptrs)
	{
		dep_checker dc;
		dc.primary = primary;
		dc.secondary = secondary;
		dc.check_flags = check_flags;
		dc.follow_aux_ptrs = follow_aux_ptrs;
		dc.follow_ptrs = follow_ptrs;
		th->walk_dependencies(obj, &dc, true);
	}
	
	bool is_valid_pointer(type_handler_i *th, const char *ptr_type)
	{
		type_handler_i *trav = th;
		while (trav)
		{
			if (!strcmp(trav->name(), ptr_type))
				return true;
			trav = trav->parent_type();
		}
		return false;
	}
	

	namespace
	{
		struct aux_resolver : public depwalker_i
		{
			db::data *db;

			bool pointer_pre(putki::instance_t *on, const char *ptr_type)
			{
				if (!*on) return false;

				if (const char *path_unres = db::is_unresolved_pointer(db, *on))
				{
					if (db::is_aux_path(path_unres))
					{
						type_handler_i *th;
						if (!db::fetch(db, path_unres, &th, on, false, true))
						{
							APP_WARNING("Could not resolve aux path [" << path_unres << "]")
						}
						else
						{
							if (!is_valid_pointer(th, ptr_type))
							{
								APP_WARNING("Have pointer to " << path_unres << ", which is of type " << th->name() << " but expected object compatible with " << ptr_type << "! Zeroing pointer.")
								*on = 0;
							}
					
							return true;
						}
					}
				}
				return false;
			}

			void pointer_post(putki::instance_t *on)
			{
				
			}
		};
	}

	void resolve_object_aux_pointers(db::data *db, const char *path)
	{
		aux_resolver dc;
		type_handler_i *th;
		instance_t obj;
		if (!db::fetch(db, path, &th, &obj, false, true))
		{
			APP_ERROR("Could not load [" << path << "]")
		}
		dc.db = db;
		th->walk_dependencies(obj, &dc, true);
	}

	namespace
	{
		struct unresolved_clearer : public depwalker_i
		{
			db::data *db;

			bool pointer_pre(putki::instance_t *on, const char *ptr_type)
			{
				if (!*on) return false;

				const char *path = db::is_unresolved_pointer(db, *on);
				if (path)
				{
					if (db::is_aux_path(path))
						APP_ERROR("I am clearing an aux path... why!")

					*on = 0;
					return false;
				}

				return true;
			}

			void pointer_post(putki::instance_t *on)
			{

			}
		};
	}

	void clear_unresolved_pointers(db::data *db, type_handler_i *th, instance_t obj)
	{
		unresolved_clearer cl;
		cl.db = db;
		th->walk_dependencies(obj, &cl, false);
	}


}
