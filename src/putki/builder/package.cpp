#include "package.h"

#include <putki/builder/typereg.h>
#include <putki/builder/db.h>
#include <putki/blob.h>

#include <map>
#include <string>
#include <vector>
#include <iostream>

namespace putki
{
	namespace package
	{
		struct entry
		{
			bool save_path;
			std::string path;
			type_handler_i *th;
			instance_t obj;
		};

		struct depwalker : putki::depwalker_i
		{
			db::data *db;
			std::vector<std::string> deps;

			virtual bool pointer_pre(instance_t * on)
			{
				if (!*on) {
					return true;
				}

				const char *path = db::pathof_including_unresolved(db, *on);
				if (!path)
				{
					std::cout << "     found OBJECT WITHOUTH PATH!" << std::endl;
					return true;
				}

				if (db::is_unresolved_pointer(db, *on))
				{
					std::cout << "Ignoring unresolved asset with path [" << path << "]" << std::endl;

					// don't traverse.
					return false;
				}

				deps.push_back(path);
				return true;
			}

			void pointer_post(instance_t *on)
			{

			}
		};

		typedef std::map<std::string, entry> blobmap_t;

		struct data
		{
			db::data *source;
			blobmap_t blobs;
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

		void add(package::data *data, const char *path, bool storepath, bool scandep)
		{
			blobmap_t::iterator i = data->blobs.find(path);
			if (i != data->blobs.end())
			{
				// adding with dep scan assumes
				if (storepath) {
					i->second.save_path = true;
				}
				return;
			}

			entry e;
			if (db::fetch(data->source, path, &e.th, &e.obj))
			{
				e.path = path;
				e.save_path = storepath;

				data->blobs[path] = e;

				// add all the dependencies of the object
				if (scandep)
				{
					depwalker dw;
					dw.db = data->source;
					e.th->walk_dependencies(e.obj, &dw, true);

					// std::cout << " * adding to package [" << path << "], pulling in " << dw.deps.size() << " dependencies" << std::endl;

					for (unsigned int i=0; i<dw.deps.size(); i++)
					{
						const bool store_path_for_dependencies = true;
						add(data, dw.deps[i].c_str(), store_path_for_dependencies, true);
					}
				}
			}
			else
			{
				std::cout << "ERROR: Trying to add [" << path << "] to package, but not found!" << std::endl;
			}
		}

		void add(data *data, const char *path, bool storepath)
		{
			add(data, path, storepath, true);
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
						std::cout << "Object [" << *p << "] will not be packed because it is missing in the db." << std::endl;
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
			std::cout << "Writing " << runtime::desc_str(rt) << " package with " << data->blobs.size() << " blobs." << std::endl;

			// create a pack list and save where each entry goes.
			std::map<std::string, int> packorder;
			std::vector<const entry*> packlist;

			for (int pass=0; pass<2; pass++)
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
			for (unsigned int i=0; i<packlist.size(); i++)
			{
				packlist[i]->th->walk_dependencies(packlist[i]->obj, &pp, true);
			}

			// change pointers and add pending strings.

			int written = 0;
			std::vector<std::string> unpacked;

			for (unsigned int i=0; i<pp.ptrs.size(); i++)
			{
				const char *path = db::pathof_including_unresolved(data->source, pp.ptrs[i].value);
				if (!path)
				{
					std::cout << "!!! POINTER NOT IN THE OUTPUT DOMAIN DETECTED !!!" << std::endl;
				}
				else
				{
					short write = 0;

					if (!packorder.count(path))
					{
						for (unsigned int i=0; i<unpacked.size(); i++)
							if (unpacked[i] == path) {
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

			std::cout << " * " << packlist.size() << " blobs, " << written << " pointer writes." << std::endl;

			char *ptr = buffer;
			char *end = buffer + available;

			ptr = pack_int32_field(ptr, packlist.size());

			for (unsigned int i=0; i<packlist.size(); i++)
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

				ptr = packlist[i]->th->write_into_buffer(rt, packlist[i]->obj, ptr, end);
				if (!ptr)
				{
					std::cout << "HELP! Buffer too small after packing " << i << " objects!" << std::endl;
				}
				else
				{
					// std::cout << "Wrote " << (ptr - start) << " bytes for slot " << i << std::endl;
				}
			}

			// Write all the pending paths that are indexed with negative numbers for ptr references.
			ptr = pack_int32_field(ptr, unpacked.size());
			for (unsigned int i=0; i<unpacked.size(); i++)
			{
				std::cout << "  => Writing path for unresolved asset " << unpacked[i] << std::endl;
				ptr = pack_int16_field(ptr, (unsigned short) unpacked[i].size() + 1);
				memcpy(ptr, unpacked[i].c_str(), unpacked[i].size() + 1);
				ptr += unpacked[i].size() + 1;
			}

			// revert all the changes!
			for (unsigned int i=0; i<pp.ptrs.size(); i++)
				*(pp.ptrs[i].ptr) = pp.ptrs[i].value;

			std::cout << "Package ready: wrote " << (ptr - buffer) << " bytes." << std::endl;

			return ptr - buffer;
		}
	}
}
