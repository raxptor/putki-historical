#include "package.h"

#include <putki/builder/typereg.h>
#include <putki/builder/db.h>
#include <putki/builder/build-db.h>
#include <putki/builder/log.h>
#include <putki/blob.h>

#include <set>
#include <map>
#include <string>
#include <vector>
#include <iostream>

namespace putki
{
	namespace package
	{
		struct preliminary
		{
			std::string path;
			bool save_path;
		};

		struct entry
		{
			bool save_path;
			std::string path;
			type_handler_i *th;
			instance_t obj;
		};

		typedef std::map<std::string, entry> blobmap_t;

		struct depwalker : putki::depwalker_i
		{
			db::data *db;
			std::set<std::string> deps;
			blobmap_t *already_added;

			virtual bool pointer_pre(instance_t * on)
			{
				if (!*on)
				{
					return true;
				}

				const char *path = db::pathof_including_unresolved(db, *on);
				if (!path)
				{
					APP_ERROR("Found object without path")
					return true;
				}

				if (db::is_unresolved_pointer(db, *on))
				{
					// fix it up
					type_handler_i *_th;
					instance_t _obj;
					if (db::fetch(db, path, &_th, &_obj))
					{
						*on = _obj;
					}
					else
					{
						APP_DEBUG("Ignoring unresolved asset with path [" << path << "]")
						return false;
					}
				}
				
				if (already_added->find(path) != already_added->end())
				{
					return false;
				}

				// already visited.
				if (deps.find(path) != deps.end())
				{
					return false;
				}

				deps.insert(path);
				return true;
			}

			void pointer_post(instance_t *on)
			{

			}
		};

		struct data
		{
			db::data *source;
			blobmap_t blobs;
			std::vector<preliminary> list;
		};

		data * create(db::data *db)
		{
			data *d = new data;
			d->source = db;
			return d;
		}

		void free(data *package)
		{
			delete package;
		}

		void debug(package::data *data, build_db::data *bdb)
		{
			blobmap_t::iterator i = data->blobs.begin();
			APP_DEBUG("Package contents:")
			std::set<std::string> included;
			std::vector<std::string> queue;

			while (i != data->blobs.end())
			{
				if (included.count(i->first))
				{
					++i;
					continue;
				}
				queue.push_back(i->first);
				included.insert(i->first);
				++i;
			}

			unsigned int pos = 0;
			while (pos < queue.size())
			{
				if (db::is_aux_path(queue[pos].c_str()))
				{
					APP_DEBUG("    + " << queue[pos])
					pos++;
					continue;
				}

				build_db::record *r = build_db::find(bdb, queue[pos].c_str());
				if (!r)
				{
					APP_ERROR("Pointer to non built object, i want to package [" << queue[pos] << "]")
					pos++;
					continue;
				}

				APP_DEBUG("  entry:" << queue[pos] << " [" << build_db::get_type(r) << "] built_sig=" << build_db::get_signature(r))

				for (int j = 0;;j++)
				{
					const char *ptr = build_db::get_pointer(r, j);
					if (!ptr)
					{
						break;
					}
					if (included.count(ptr))
					{
						continue;
					}
					APP_DEBUG("    pointer to [" << ptr << "]")
					queue.push_back(ptr);
					included.insert(ptr);
				}

				pos++;
			}
		}

		void add(package::data *data, const char *path, std::vector<std::string> *bulkadd, bool storepath, bool scandep)
		{
			if (path)
			{
				APP_DEBUG("Adding single [" << path << "] to package")
			}
			else if (bulkadd)
			{
				if (bulkadd->empty())
				{
					return;
				}
				APP_DEBUG("Adding " << bulkadd->size() << " elements in bulk")
			}
			else
			{
				APP_ERROR("Both path and bulkadd are null")
			}

			for (unsigned int k = 0;k < 1 || (bulkadd && k < bulkadd->size());k++)
			{
				if (bulkadd)
				{
					path = (*bulkadd)[k].c_str();
				}
				blobmap_t::iterator i = data->blobs.find(path);
				if (i != data->blobs.end())
				{
					// adding with dep scan assumes
					if (storepath)
					{
						i->second.save_path = true;
					}
					if (bulkadd)
					{
						bulkadd->erase(bulkadd->begin() + k);
						i--;
					}
					else
					{
						return;
					}
				}
			}

			for (unsigned int i = 0;i < 1 || (bulkadd && i < bulkadd->size());i++)
			{
				if (bulkadd)
				{
					path = (*bulkadd)[i].c_str();
				}

				entry e;
				if (db::fetch(data->source, path, &e.th, &e.obj))
				{
					e.path = path;
					e.save_path = storepath;
					data->blobs[path] = e;
				}
				else
				{
					APP_ERROR("Trying to add [" << path << "] to package, but not found!")
					return;
				}
			}

			if (scandep)
			{
				depwalker dw;
				dw.already_added = &data->blobs;
				dw.db = data->source;

				for (unsigned int k = 0;k < 1 || (bulkadd && k < bulkadd->size());k++)
				{
					if (bulkadd)
					{
						path = (*bulkadd)[k].c_str();
					}

					data->blobs[path].th->walk_dependencies(data->blobs[path].obj, &dw, true);

					std::set<std::string>::iterator i = dw.deps.begin();
					std::vector<std::string> next_add;
					while (i != dw.deps.end())
					{
						next_add.push_back(*i++);
					}

					const bool store_path_for_dependencies = true;
					add(data, 0, &next_add, store_path_for_dependencies, true);
				}
			}
		}

		void add(data *data, const char *path, bool storepath)
		{
			preliminary p;
			p.path = path;
			p.save_path = storepath;
			data->list.push_back(p);
		}

		const char *get_needed_asset(data *d, unsigned int i)
		{
			if (i < d->list.size())
			{
				return d->list[i].path.c_str();
			}
			return 0;
		}

		// extracts all the pointer values and their values so packaging can
		// temporarily rewrite them.
		struct pointer_rewriter : putki::depwalker_i
		{
			struct entry
			{
				instance_t *ptr;
				instance_t value;
			};

			db::data *db;
			std::vector<entry> ptrs;

			bool pointer_pre(instance_t *p)
			{
				if (*p) // don't modify null pointer
				{
					// save what we did so we can undo later
					entry e;
					e.ptr = p;
					e.value = *p;
					ptrs.push_back(e);

					const char *path = db::pathof(db, *p);
					if (!path)
					{
						APP_WARNING("Object [" << *p << "] will not be packed because it is missing in the db.")
						// need to return false because this could be unresolved. we can't do anything about it anyway!.
						return false;
					}
				}

				return true;
			}

			void pointer_post(instance_t *p)
			{

			}

		};

		long write(data *data, runtime::descptr rt, char *buffer, long available)
		{
			for (unsigned int i = 0;i < data->list.size();i++)
				add(data, data->list[i].path.c_str(), 0, data->list[i].save_path, true);
			data->list.clear();

			APP_DEBUG("Writing " << runtime::desc_str(rt) << " package with " << data->blobs.size() << " blobs.")

			// create a pack list and save where each entry goes.
			std::map<std::string, int> packorder;
			std::vector<const entry*> packlist;

			for (int pass = 0;pass < 2;pass++)
			{
				blobmap_t::const_iterator i = data->blobs.begin();
				while (i != data->blobs.end())
				{
					// store all with paths first
					if ((pass == 0) == i->second.save_path)
					{
						packorder[i->first] = packlist.size();
						packlist.push_back(&(i->second));
					}
					++i;
				}
			}

			// the packlist is now doomed if we manipulate with the blobmap
			// pack all pointers so they point into the slot list.
			pointer_rewriter pp;
			pp.db = data->source;

			// get all pointerts rewritten to be pure indices.
			for (unsigned int i = 0;i < packlist.size();i++)
			{
				packlist[i]->th->walk_dependencies(packlist[i]->obj, &pp, true);
			}

			// change pointers and add pending strings.

			int written = 0;
			std::vector<std::string> unpacked;

			for (unsigned int i = 0;i < pp.ptrs.size();i++)
			{
				const char *path = db::pathof_including_unresolved(data->source, pp.ptrs[i].value);
				if (!path)
				{
					APP_ERROR("Pointer not in output db")
				}
				else
				{
					short write = 0;

					if (!packorder.count(path))
					{
						for (unsigned int i = 0;i < unpacked.size();i++)
							if (unpacked[i] == path)
							{
								write = -(int)i - 1;
							}

						if (!write)
						{
							unpacked.push_back(path);
							write = -((int)unpacked.size());
						}
					}
					else
					{
						write = 1 + packorder[path];
						// std::cout << " " << path << " => slot " << write << std::endl;
					}

					// clear whole field.
					*(pp.ptrs[i].ptr) = 0;
					*((short*)pp.ptrs[i].ptr) = write;

					++written;
				}
			}

			APP_DEBUG(" * " << packlist.size() << " blobs, " << written << " pointer writes.")

			char *ptr = buffer;
			char *end = buffer + available;

			ptr = pack_int32_field(ptr, packlist.size());

			for (unsigned int i = 0;i < packlist.size();i++)
			{
				const unsigned int has_path_flag = 1 << 31;
				if (packlist[i]->save_path)
				{
					ptr = pack_int32_field(ptr, packlist[i]->th->id() | has_path_flag);

					/// write string prefixed with int16 length
					ptr = pack_int16_field(ptr, (unsigned short) packlist[i]->path.size() + 1);
					memcpy(ptr, packlist[i]->path.c_str(), packlist[i]->path.size() + 1);
					ptr += packlist[i]->path.size() + 1;
				}
				else
				{
					ptr = pack_int32_field(ptr, packlist[i]->th->id());
				}

				char *start = ptr;
				ptr = packlist[i]->th->write_into_buffer(rt, packlist[i]->obj, ptr, end);
				if (!ptr)
				{
					std::cout << "HELP! Wrote 0 bytes after packing " << i << " objects!" << std::endl;
					std::cout << "  - Buffer could be too small (" << available << " bytes)" << std::endl;
					std::cout << "  - Writer could fail because output platform not recognized" << std::endl;
					std::cout << "Attempted to write_into_buffer on " << packlist[i]->th->name() << std::endl;
					APP_ERROR("HELP")
				}
			}

			// Write all the pending paths that are indexed with negative numbers for ptr references.
			ptr = pack_int32_field(ptr, unpacked.size());
			for (unsigned int i = 0;i < unpacked.size();i++)
			{
				APP_DEBUG("Writing path for unresolved asset " << unpacked[i])
				ptr = pack_int16_field(ptr, (unsigned short) unpacked[i].size() + 1);
				memcpy(ptr, unpacked[i].c_str(), unpacked[i].size() + 1);
				ptr += unpacked[i].size() + 1;
			}

			// revert all the changes!
			for (unsigned int i = 0;i < pp.ptrs.size();i++)
				*(pp.ptrs[i].ptr) = pp.ptrs[i].value;

			APP_DEBUG("Package ready: wrote " << (ptr - buffer) << " bytes.")

			return ptr - buffer;
		}
	}
}
