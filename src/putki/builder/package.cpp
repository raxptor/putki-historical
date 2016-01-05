#include "package.h"
#include "tok.h"

#include <putki/builder/typereg.h>
#include <putki/builder/db.h>
#include <putki/builder/build-db.h>
#include <putki/builder/log.h>
#include <putki/blob.h>

extern "C"
{
#include <md5/md5.h>
}

#include <set>
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

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
			
			int pack_slot_index;
			int file_index, file_slot_index;
			unsigned int ofs_begin;
			unsigned int ofs_end;
			std::string bytes_signature;
			build_db::record *r;
		};
		
		struct manifest_pointer
		{
			std::string path;
			int slot_index;
		};
		
		struct manifest_slot
		{
			std::string type;
			std::string signature;
			std::string file;
			std::string path;
			int begin, end;
			std::vector<int> deps;
		};
		
		typedef std::map<std::string, int> path2slot_t;
		
		struct previous_pkg
		{
			std::string path;
			std::string file;
			path2slot_t path_to_slot;
			std::vector<manifest_slot> slots;
		};

		typedef std::map<std::string, previous_pkg> previous_t;
		typedef std::map<std::string, entry> blobmap_t;
		
		struct use_previous
		{
			previous_pkg *previous;
			std::vector<int> slots_i_want;
			std::map<int, int> slot_remapping;
		};

		struct data
		{
			db::data *source;
			blobmap_t blobs;
			previous_t previous;
			std::vector<preliminary> list;
			std::vector<use_previous> previous2use;
		};
		
		void compute_previous_slot_mapping(data *target, use_previous *out)
		{
			for (int i=0;i!=out->slots_i_want.size();i++)
			{
				int slot_index = out->slots_i_want[i];
				manifest_slot & slot = out->previous->slots[slot_index];
				
				for (int j=0;j!=slot.deps.size();j++)
				{
					manifest_slot & depslot = out->previous->slots[slot.deps[j]];
					blobmap_t::iterator m = target->blobs.find(depslot.path);
					if (m == target->blobs.end())
					{
						APP_WARNING("Could not produce remapping for extdep " << depslot.path << " in package " << out->previous->file);
						APP_WARNING("I originally wanted to pack " << slot.path << " and this was a dependency.")
						continue;
					}
					
					// don't remap same slots (waste of space).
					if (slot.deps[j] == m->second.pack_slot_index)
						continue;
					
					out->slot_remapping[slot.deps[j]] = m->second.pack_slot_index;
				}
			}
			
			std::map<int, int>::iterator i = out->slot_remapping.begin();
			while (i != out->slot_remapping.end())
			{
				APP_DEBUG("Import " << out->previous->file << " remap [" << i->first << "] => [" << i->second << "]")
				++i;
			}
		}
		
		void add_previous_package(package::data *data, const char *basepath, const char *path)
		{
			previous_t::iterator i = data->previous.find(path);
			if (i != data->previous.end())
				return;
		
			std::string real_path(basepath);
			real_path.append("/");
			real_path.append(path);
			
			APP_DEBUG("Loading previous package " << path << " (" << real_path << ")");
	
			std::string manifest_name = (real_path + ".manifest");

			previous_pkg tmppkg;
			data->previous.insert(previous_t::value_type(path, tmppkg));
			i = data->previous.find(path);
		
			previous_pkg *pkg = &i->second;
			manifest_slot *slot = 0;
			
			pkg->file = path;

			tok::data *mf = tok::load(manifest_name.c_str());
			if (mf)
			{
				tok::tokenize_newlines(mf);
				// educated guess
				pkg->slots.reserve(tok::size(mf) / 2 + 10);
			}

			int index = 0;
			while (mf)
			{
				const char *ln = tok::get(mf, index++);
				if (!ln)
					break;
					
				std::string line(ln);
					
				if (line[0] == 'p')
				{
					if (slot)
					{
						line.erase(0, 2);
						int split = line.find_first_of(':');
						slot->deps.push_back(
							atoi(
								line.substr(0, split).c_str()
							)
						);
					}
					else
					{
						APP_WARNING("Manifest is malformatted because slot = null when i get p:");
					}
				}
				else if (line[0] == '#')
				{
					manifest_slot tmp;
					
					int split = line.find_first_of(':');
					std::string slotnum = line.substr(1, split - 1);
					int slotidx = atoi(slotnum.c_str());
					if (slotidx != pkg->slots.size())
					{
						APP_WARNING("Slot index mis-match at " << slotidx << " but " << pkg->slots.size());
					}
					pkg->slots.push_back(tmp);
					slot = &pkg->slots.back();
					
					line.erase(0, split + 1);
					for (int i=0;i<6;i++)
					{
						split = line.find_first_of(':');
						std::string value;
						if (split == std::string::npos)
						{
							value = line;
						}
						else
						{
							value = line.substr(0, split);
							line.erase(0, split + 1);
						}
						
						switch (i)
						{
							case 0:
								slot->type = value;
								break;
							case 1:
								slot->path = value;
								break;
							case 2:
								slot->signature = value;
								break;
							case 3:
								slot->file = value;
								if (value != "!self")
								{
									add_previous_package(data, basepath, value.c_str());
								}
								break;
							case 4:
								slot->begin = atoi(value.c_str());
								break;
							case 5:
								slot->end = atoi(value.c_str());
								break;
						}
					}
				}
			}
			
			// make mapping table
			for (unsigned int i=0;i<pkg->slots.size();i++)
				pkg->path_to_slot[pkg->slots[i].path] = i;

			tok::free(mf);
			APP_DEBUG("Loaded previous package with " << pkg->slots.size() << " slots!")
		}
		

		struct depwalker : putki::depwalker_i
		{
			db::data *db;
			std::set<std::string> deps;
			blobmap_t *already_added;

			virtual bool pointer_pre(instance_t * on, const char *ptr_type)
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

				type_handler_i *_th = 0;
				instance_t _obj = 0;

				if (db::is_unresolved_pointer(db, *on))
				{
					// fix it up
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

				if (_th || db::fetch(db, path, &_th, &_obj))
				{
					if (!_th->in_output())
						return false;
				}

				deps.insert(path);
				return true;
			}

			void pointer_post(instance_t *on)
			{

			}
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
		
		bool pick_from_previous(package::data *data, const char *path, const char *type, const char *signature, entry *fill)
		{
			previous_t::iterator p = data->previous.begin();
			while (p != data->previous.end())
			{
				// any with this path?
				path2slot_t::iterator m = p->second.path_to_slot.find(path);
				if (m == p->second.path_to_slot.end())
				{
					p++;
					continue;
				}
			
				// match type & built sig
				manifest_slot *slot = &p->second.slots[m->second];
				
				// not matching always on type, when aux & sig match then manifest contains the type.
				if ((type && strcmp(slot->type.c_str(), type)) || strcmp(slot->signature.c_str(), signature))
				{
					p++;
					continue;
				}
				
				// ok we can use this.
				use_previous *prev = 0;
				for (int i=0;i!=data->previous2use.size();i++)
				{
					if (data->previous2use[i].previous == &p->second)
					{
						prev = &data->previous2use[i];
						fill->file_index = i;
						fill->file_slot_index = m->second;
						break;
					}
				}
				
				if (!prev)
				{
					use_previous up;
					up.previous = &p->second;
					data->previous2use.push_back(up);
					prev = &data->previous2use.back();
					
					fill->file_index = data->previous2use.size() - 1;
					fill->file_slot_index = m->second;
				}
				
				fill->ofs_begin = slot->begin;
				fill->ofs_end = slot->end;
				fill->th = typereg_get_handler(slot->type.c_str());
				
				APP_DEBUG("Found match in " << p->first << " in slot " << m->second << " for [" << path << "]")
				prev->slots_i_want.push_back(m->second);

				return true;
			}
			return false;
		}

		void add(package::data *data, const char *path, std::vector<std::string> *bulkadd, bool storepath, bool scandep, build_db::data *bdb)
		{
			// should be false but is interesting until package manifest is
			// implemented
			const bool store_path_for_dependencies = true;
		
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

			// filter away those already added.
			for (unsigned int k = 0;k < 1 || (bulkadd && k < bulkadd->size());k++)
			{
				const char *addpath = path;
		
				if (bulkadd)
				{
					if (bulkadd->empty())
						break;
		
					addpath = (*bulkadd)[k].c_str();
				}
				
				blobmap_t::iterator i = data->blobs.find(addpath);
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
			
			if (bulkadd && bulkadd->empty())
				return;
			
			// verify that all exist, we can error out here completely.
			
			for (unsigned int i = 0;i < 1 || (bulkadd && i < bulkadd->size());i++)
			{
				const char *addpath = path;
		
				if (bulkadd)
				{
					if (bulkadd->empty())
						break;

					addpath = (*bulkadd)[i].c_str();
				}
				
				if (!db::exists(data->source, addpath))
				{
					APP_WARNING("Trying to add [" << addpath << "] to package, but not found in db output!")
					
					if (!bulkadd)
						return;

					bulkadd->erase(bulkadd->begin() + i);
					i--;
					continue;
				}
				
				build_db::record *r = build_db::find(bdb, addpath);
				if (!r)
				{
					if (!db::is_aux_path(addpath))
						APP_ERROR("Item exists in output db but not in build_db!")
				}
				
				entry e;
				e.path = addpath;
				e.save_path = storepath;
		
				// Now is time to check if it can be picked from an old manifest.
				if (r && pick_from_previous(data, addpath, build_db::get_type(r), build_db::get_signature(r), &e))
				{
					data->blobs[addpath] = e;
		
					// now it might have a few auxes, then they are in the source too.
					int p = 0;
					
					std::vector<std::string> deps_to_add;
					while (true)
					{
						const char *auxpath = build_db::get_pointer(r, p++);
						if (!auxpath)
							break;
						
						if (!db::is_aux_path(auxpath))
						{
							build_db::record * tr = build_db::find(bdb, auxpath);
							if (!tr)
							{
								APP_ERROR("No own record for " << auxpath)
								continue;
							}
							
							if (!typereg_get_handler(build_db::get_type(tr))->in_output())
								continue;
	
							APP_DEBUG("Adding deps to include [" << auxpath << "]")
							deps_to_add.push_back(auxpath);
							continue;
						}
												
						entry e;
						e.path = auxpath;
						e.save_path = storepath;
						if (pick_from_previous(data, auxpath, 0, build_db::get_signature(r), &e))
							data->blobs[auxpath] = e;
					}
					
					if (!deps_to_add.empty())
						add(data, 0, &deps_to_add, store_path_for_dependencies, true, bdb);
				
					if (bulkadd)
					{
						bulkadd->erase(bulkadd->begin() + i);
						i--;
						continue;
					}
					else
					{
						return;
					}
				}
				
				// add entry.
				e.path = addpath;
				e.ofs_begin = 0;
				e.ofs_end = 0;
				e.file_index = e.file_slot_index = -1;
			
				if (!db::fetch(data->source, addpath, &e.th, &e.obj))
					APP_ERROR("db::exist said " << addpath << " exists, but it could not be loaded!")

				data->blobs[addpath] = e;
			}
			
			if (bulkadd && bulkadd->empty())
				return;
			
			// All that remains are objects that could not be loaded from previous file,
			// so these must be scanned here. We could use the build record pointer list...
			// but not much gain?

			depwalker dw;
			dw.already_added = &data->blobs;
			dw.db = data->source;

			for (unsigned int k = 0;k < 1 || (bulkadd && k < bulkadd->size());k++)
			{
				const char *addpath = path;
				
				if (bulkadd)
				{
					addpath = (*bulkadd)[k].c_str();
				}

				// run the walk_dependencies fn always to make sure pointers are resolved.
				data->blobs[addpath].th->walk_dependencies(data->blobs[addpath].obj, &dw, true);

				if (scandep)
				{
					std::set<std::string>::iterator i = dw.deps.begin();
					std::vector<std::string> next_add;
					while (i != dw.deps.end())
					{
						next_add.push_back(*i++);
					}

					add(data, 0, &next_add, store_path_for_dependencies, true, bdb);
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

			bool pointer_pre(instance_t *p, const char *ptr_type)
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

		long write(data *data, runtime::descptr rt, char *buffer, long available, build_db::data *build_db, sstream & manifest)
		{
			for (unsigned int i = 0;i < data->list.size();i++)
				add(data, data->list[i].path.c_str(), 0, data->list[i].save_path, true, build_db);

			APP_DEBUG("Writing " << runtime::desc_str(rt) << " package with " << data->blobs.size() << " blobs.")

			// create a pack list and save where each entry goes.
			std::map<std::string, int> packorder;
			std::vector<entry*> packlist;

			// we put the data that was asked for first.
			for (int k=0;k!=data->list.size();k++)
			{
				if (!data->list[k].save_path)
					continue;

				blobmap_t::iterator i = data->blobs.find(data->list[k].path);
				if (i == data->blobs.end())
					continue;

				packorder[i->first] = packlist.size();
				i->second.pack_slot_index = packlist.size();
				packlist.push_back(&(i->second));
			}

			// then the rest
			blobmap_t::iterator i = data->blobs.begin();
			while (i != data->blobs.end())
			{
				if (packorder.find(i->first) == packorder.end())
				{
					packorder[i->first] = packlist.size();
					i->second.pack_slot_index = packlist.size();
					packlist.push_back(&(i->second));
				}
				++i;
			}

			data->list.clear();
			
			// Go through all the pointers in the object, writing slot indices (as +1 though as 0=0)
			// where the unpacked slots end up as the last ones after the one in the packlist.
			

			// the packlist is now doomed if we manipulate with the blobmap
			// pack all pointers so they point into the slot list.
			pointer_rewriter pp;
			pp.db = data->source;

			// get all pointers rewritten to be pure indices.
			APP_DEBUG("Converting pointers into indices and finding unresolved pointers...")
			for (unsigned int i = 0;i < packlist.size();i++)
			{
				if (packlist[i]->file_slot_index == -1)
					packlist[i]->th->walk_dependencies(packlist[i]->obj, &pp, true, true);
			}
			
			int written = 0;
			std::vector<std::string> unpacked;
			for (unsigned int i = 0;i < pp.ptrs.size();i++)
			{
				const char *path = db::pathof_including_unresolved(data->source, pp.ptrs[i].value);
				if (!path)
				{
					APP_ERROR("Pointer not in output db")
					continue;
				}
				
				short write = 0;
				if (!packorder.count(path))
				{
					for (unsigned int i = 0;i < unpacked.size();i++)
						if (unpacked[i] == path)
							write = (short)(packlist.size() + i + 1);

					if (!write)
					{
						write = (short)(packlist.size() + unpacked.size() + 1);
						unpacked.push_back(path);
					}
				}
				else
				{
					write = 1 + packorder[path];
				}

				// clear whole field.
				*(pp.ptrs[i].ptr) = 0;
				*((short*)pp.ptrs[i].ptr) = write;

				++written;
			}
		
			APP_DEBUG("In pack list: " << packlist.size() << ", unresolved:" << unpacked.size())
			
			// --- Write package information
			char *ptr = buffer;
			char *end = buffer + available;

			// PTKP
			const unsigned int header = 0x504B5450;
			const unsigned int flags = 0x0;
			
			ptr = pack_int32_field(ptr, header);
			ptr = pack_int32_field(ptr, flags);
			
			char *header_size_pos = ptr;
			ptr = pack_int32_field(ptr, 0); // size of header
			ptr = pack_int32_field(ptr, 0); // size of all data
			
			// File import list
			ptr = pack_int16_field(ptr, (short)data->previous2use.size());
			for (int i=0;i!=data->previous2use.size();i++)
			{
				use_previous *use = &data->previous2use[i];
				previous_pkg *prev = use->previous;
				
				compute_previous_slot_mapping(data, use);

				const char *name = prev->file.c_str();
				const size_t len = prev->file.size() + 1;
				ptr = pack_int16_field(ptr, (short)len);
				ptr = pack_int16_field(ptr, (short)use->slot_remapping.size());
				
				memcpy(ptr, name, len);
				ptr += len;
				
				std::map<int, int>::iterator j = use->slot_remapping.begin();
				while (j != use->slot_remapping.end())
				{
					ptr = pack_int16_field(ptr, (short)j->first);
					ptr = pack_int16_field(ptr, (short)j->second);
					j++;
				}
			}
			
			APP_DEBUG("File import list: " << (ptr - buffer) << " bytes.")
			
			std::vector<char*> filepospos;

			// Now comes slot list, we add both packed & unpacked.
			ptr = pack_int16_field(ptr, packlist.size() + unpacked.size());
			for (int i=0;i!=(packlist.size() + unpacked.size());i++)
			{
				const int PKG_FLAG_PATH       = 1;
				const int PKG_FLAG_EXTERNAL   = 2;
				const int PKG_FLAG_INTERNAL   = 4;
				const int PKG_FLAG_UNRESOLVED = 8;
				
				const char *path;
				unsigned short flags = 0;
				
				if (i < packlist.size())
				{
					path = packlist[i]->path.c_str();
			
					if (packlist[i]->save_path)
						flags |= PKG_FLAG_PATH;
					if (packlist[i]->file_index != -1)
						flags |= PKG_FLAG_EXTERNAL;
					else
						flags |= PKG_FLAG_INTERNAL;
				}
				else
				{
					path = unpacked[i - packlist.size()].c_str();
					flags |= PKG_FLAG_PATH;
					flags |= PKG_FLAG_UNRESOLVED;
				}
				
				// path if wanted.
				ptr = pack_int16_field(ptr, flags);
				if (flags & PKG_FLAG_PATH)
				{
					ptr = pack_int16_field(ptr, strlen(path) + 1);
					memcpy(ptr, path, strlen(path) + 1);
					ptr += strlen(path) + 1;
				}
				
				// we come back later to fill these in!
				if (flags & PKG_FLAG_EXTERNAL)
				{
					filepospos.push_back(0);
					ptr = pack_int16_field(ptr, packlist[i]->file_index);
					ptr = pack_int16_field(ptr, packlist[i]->file_slot_index);
					ptr = pack_int16_field(ptr, packlist[i]->th->id());
					ptr = pack_int32_field(ptr, packlist[i]->ofs_begin);
					ptr = pack_int32_field(ptr, packlist[i]->ofs_end);
					
				}
				else if (flags & PKG_FLAG_INTERNAL)
				{
					filepospos.push_back(ptr);
					ptr = pack_int32_field(ptr, 0);
					ptr = pack_int32_field(ptr, 0);
					ptr = pack_int16_field(ptr, packlist[i]->th->id());
				}
			}
			
			APP_DEBUG("Total header is " << (ptr - buffer) << " bytes.")
			header_size_pos = pack_int32_field(header_size_pos, ptr - buffer);
		
			int total_loaded_data_size = 0;
			
			// Write actual slot content
			for (unsigned int i = 0;i < packlist.size();i++)
			{
				char *start = ptr;
				
				if (packlist[i]->file_slot_index == -1)
				{
					ptr = packlist[i]->th->write_into_buffer(rt, packlist[i]->obj, ptr, end);
					if (!ptr)
					{
						APP_WARNING("HELP! Wrote 0 bytes after packing " << i << " objects!")
						APP_WARNING("  - Buffer could be too small (" << available << " bytes)")
						APP_WARNING("  - Writer could fail because output platform not recognized")
						APP_WARNING("Attempted to write_into_buffer on " << packlist[i]->th->name())
						APP_ERROR("HELP")
						packlist[i]->ofs_begin = 0;
						packlist[i]->ofs_end = 0;
						continue;
					}
					
					packlist[i]->ofs_begin = start - buffer;
					packlist[i]->ofs_end = ptr - buffer;
					
					// fill in with start & end offsets in this file.
					char *tmp_ptr = filepospos[i];
					tmp_ptr = pack_int32_field(tmp_ptr, start - buffer);
					tmp_ptr = pack_int32_field(tmp_ptr, ptr - buffer);
					total_loaded_data_size += ptr - start;
				}
				else
				{
					// this comes from when it was added from external resource
					total_loaded_data_size += (packlist[i]->ofs_end - packlist[i]->ofs_begin);
				}
			
				build_db::record *r = 0;
				
				char buf[2048];
				if (db::is_aux_path(packlist[i]->path.c_str()))
				{
					db::base_asset_path(packlist[i]->path.c_str(), buf, sizeof(buf));
					r = build_db::find(build_db, buf);
				}
				else
				{
					r = build_db::find(build_db, packlist[i]->path.c_str());
				}
				
				if (!r)
				{
					APP_ERROR("Could not grab signature for " << packlist[i]->path << "!")
				}

				// write manifest entry
				manifest << "#" << i << ":" << packlist[i]->th->name() << ":" <<
					   packlist[i]->path << ":" << build_db::get_signature(r) << ":";
				
				if (packlist[i]->file_slot_index == -1)
					manifest << "!self";
				else
					manifest << data->previous2use[packlist[i]->file_index].previous->file;
					
				manifest << ":" << packlist[i]->ofs_begin << ":" << packlist[i]->ofs_end << "\n";
					   
				int p = 0;
				while (r)
				{
					const char *ptr = build_db::get_pointer(r, p);
					if (ptr)
					{
						blobmap_t::iterator b = data->blobs.find(ptr);
						if (b != data->blobs.end())
						{
							manifest << "p:" << packorder[ptr] << ":" << ptr << "\n";
						}
						else
						{
							manifest << "p:?:" << ptr << "\n";
						}
						p++;
					}
					else
					{
						break;
					}
				}
			}
			
			// compute total size
			pack_int32_field(header_size_pos, total_loaded_data_size);

			// Revert all the changes!
			for (unsigned int i = 0;i < pp.ptrs.size();i++)
				*(pp.ptrs[i].ptr) = pp.ptrs[i].value;

			APP_DEBUG("Package ready: wrote " << (ptr - buffer) << " bytes in total.")

			return ptr - buffer;
		}
	}
}
