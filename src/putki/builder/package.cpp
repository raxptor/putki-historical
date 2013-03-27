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
			
			virtual void pointer(instance_t * on)
			{
				const char *path = db::pathof(db, *on);
				if (!path)
					std::cout << "     found OBJECT WITHOUTH PATH!" << std::endl;
					
				deps.push_back(path);
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
				if (storepath)
					i->second.save_path = true;
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
					e.th->walk_dependencies(e.obj, &dw);
					
					std::cout << " * adding to package [" << path << "], pulling in " << dw.deps.size() << " dependencies" << std::endl;

					for (unsigned int i=0;i<dw.deps.size();i++)
						add(data, dw.deps[i].c_str(), false, false);
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
		
		long inbytes(data *d)
		{
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
			
			void pointer(instance_t *p)
			{
				const char *path = db::pathof(db, *p);
				if (!path)
				{
					std::cout << "Found unpackable dangling object [" << *p << "]" << std::endl;
					return;
				}
				else if (*p) // don't modify null pointer
				{
					// save what we did so we can undo later
					entry e;
					e.ptr = p;
					e.value = *p;
					ptrs.push_back(e);
				}
			}
		};
		
		long write(data *data, putki::runtime rt, char *buffer, long available)
		{
			std::cout << "Writing package with " << data->blobs.size() << " blobs." << std::endl;
			
			// create a pack list and save where each entry goes.
			std::map<std::string, int> packorder;
			std::vector<const entry*> packlist;
			
			blobmap_t::const_iterator i = data->blobs.begin();
			while (i != data->blobs.end())
			{
				packorder[i->first] = packlist.size();
				packlist.push_back(&(i->second));
				++i;
			}
			
			// the packlist is now doomed if we manipulate with the blobmap
			// pack all pointers so they point into the slot list.
			pointer_rewriter pp;
			pp.db = data->source;
			
			// get all pointerts rewritten to be pure indices.
			for (unsigned int i=0;i<packlist.size();i++)
			{
				packlist[i]->th->walk_dependencies(packlist[i]->obj, &pp);
			}
			
			// change pointers and add pending strings.

			int written = 0;
			std::vector<std::string> unpacked;
			
			for (unsigned int i=0;i<pp.ptrs.size();i++)
			{
				const char *path = db::pathof(data->source, pp.ptrs[i].value);
				if (!path)
				{
					std::cout << "Un-packed asset [" << path << "]" << std::endl;
				}
				else
				{
					short write = 0;
								
					if (!packorder.count(path))
					{
						for (unsigned int i=0;i<unpacked.size();i++)
							if (unpacked[i] == path)
								write = - (int)i - 1;

						if (!write)
						{
							unpacked.push_back(path);
							write = - ((int)unpacked.size());
						}
					}
					else
					{
						write = packorder[path];
					}
					
					// clear whole field.
					*(pp.ptrs[i].ptr) = 0;
					*((short*)pp.ptrs[i].ptr) = write;
				
					std::cout << " " << path << " => slot " << write << std::endl;
					++written;
				}
			}
			
			std::cout << " * Did " << written << " pointer writes." << std::endl;
			
			char *ptr = buffer;
			char *end = buffer + available;

			ptr = pack_int32_field(ptr, packlist.size());
			
			for (unsigned int i=0;i<packlist.size();i++)
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
					std::cout << "HELP! Buffer too small after packing " << i << " objects!" << std::endl;
				}
				else
				{
					std::cout << "Wrote " << (ptr - start) << " bytes for slot " << i << std::endl;
				}
			}

			// Write all the pending paths that are indexed with negative numbers for ptr references.
			ptr = pack_int32_field(ptr, unpacked.size());
			for (unsigned int i=0;i<unpacked.size();i++)
			{
				std::cout << "  => Writing path for unresolved asset " << unpacked[i] << std::endl;
				ptr = pack_int16_field(ptr, (unsigned short) unpacked[i].size() + 1);
				memcpy(ptr, unpacked[i].c_str(), unpacked[i].size() + 1);
				ptr += unpacked[i].size() + 1;
			}
			
			// revert all the changes!
			for (unsigned int i=0;i<pp.ptrs.size();i++)
				*(pp.ptrs[i].ptr) = pp.ptrs[i].value;
			
			return ptr - buffer;
		}
	}
}