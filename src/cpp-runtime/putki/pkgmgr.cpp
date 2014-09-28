
#include "pkgmgr.h"
#include "types.h"
#include "blob.h"
#include "liveupdate/liveupdate.h"
#include "log/log.h"

#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstddef>

namespace putki
{
	namespace pkgmgr
	{
		static const int PKG_FLAG_PATH       = 1;
		static const int PKG_FLAG_EXTERNAL   = 2;
		static const int PKG_FLAG_INTERNAL   = 4;
		static const int PKG_FLAG_UNRESOLVED = 8;
	
		struct package_slot
		{
			const char *path;
			instance_t obj, obj_end;
			int16_t flags, type_id;
			int16_t file_index, file_slot_index;
		};

		struct loaded_package
		{
			bool should_free;
			char *beg, *end;
			package_slot *slots;
			unsigned int slots_size;
			const char **unresolved;
			unsigned int unresolved_size;
		};

		struct pkg_ptrs : public depwalker_i
		{
			struct entry
			{
				instance_t *ptr;
				short index;
			};

			std::vector<entry> entries;

			bool pointer_pre(instance_t *ptr)
			{
				entry e;
				e.ptr = ptr;
				e.index = *((unsigned short *)ptr);

				entries.push_back(e);

				// don't traverse at all.
				return false;
			}

			void pointer_post(instance_t *ptr)
			{

			}
		};

		struct resolve_status
		{
			pkg_ptrs ptrs;
		};

		resolve_status *alloc_resolve_status()
		{
			return new resolve_status();
		}

		void free_resolve_status(resolve_status *p)
		{
			delete p;
		}

		void free_on_release(loaded_package *p)
		{
			p->should_free = true;
		}

		// returns number of unresolved pointers remaining.
		int resolve_pointers_with(loaded_package *target, resolve_status *s, loaded_package *aux)
		{
			int resolved = 0, unresolved = 0;
			for (unsigned int i=0; i<s->ptrs.entries.size(); i++)
			{
				pkg_ptrs::entry &e = s->ptrs.entries[i];

				unsigned int path_index = (-e.index) - 1;
				if (path_index < target->unresolved_size)
				{
					instance_t nw = resolve(aux, target->unresolved[path_index]);
					if (nw)
					{
						resolved++;
						(*e.ptr) = nw;
					}
				}

				if (e.index < 0 && !*e.ptr) {
					unresolved++;
				}
			}

			return unresolved;
		}
		
		uint16_t parse_int16(char **src)
		{
			uint16_t *ptr = (uint16_t*) *src;
			(*src) += 2;
			return *ptr;
		}
		
		uint32_t parse_int32(char **src)
		{
			uint32_t *ptr = (uint32_t*) *src;
			(*src) += 4;
			return *ptr;
		}

		// look at the first bytes and say if valid and how big the header is.
		bool get_header_info(char *beg, char *end, uint32_t *total_header_size, uint32_t *total_data_size)
		{		
			if (end - beg < 16)
				return false;
								
			beg += 8; // skip header & flags
			
			*total_header_size = parse_int32(&beg);
			*total_data_size = parse_int32(&beg);
			return true;
		}
		
		// parse from buffer
		loaded_package * parse(char *header, char *data, resolve_status *out)
		{
			char *hdr_rp = header;
			const int16_t max_imports = 256;
		
			// grab headers.
			/* const int32_t hdr_tag = */ parse_int32(&hdr_rp);
			/* const int32_t flags = */ parse_int32(&hdr_rp);
			const int32_t hdr_sz = parse_int32(&hdr_rp);
			const int32_t data_sz = parse_int32(&hdr_rp);
			
			const int16_t num_imports = parse_int16(&hdr_rp);
			
			if (num_imports > max_imports)
			{
				PTK_ERROR("num_imports too large! bump max_imports or fix the problem otherwise")
				return 0;
			}
			
			struct import
			{
				char *import_path;
				char *remap_table;
				uint16_t remaps_count;
			} parsed_imports[max_imports];
						
			for (int16_t i=0;i!=num_imports;i++)
			{
				import *imp = &parsed_imports[i];
				
				uint16_t name_length = parse_int16(&hdr_rp);
				imp->remaps_count = parse_int16(&hdr_rp);
				
				imp->import_path = hdr_rp;
				imp->remap_table = hdr_rp + name_length;
				
				hdr_rp += name_length;
				hdr_rp += 2 * imp->remaps_count;
				
				PTK_DEBUG("Import " << i << ", name:" << imp->import_path << " remaps:" << imp->remap_table);
			}
			
			PTK_DEBUG("File has " << num_imports << " external imports, and it will be " << data_sz << " when loaded");
			PTK_DEBUG("Throw-away header is " << hdr_sz << " bytes")
			
			int slot_count = parse_int16(&hdr_rp);
			
			loaded_package *lp = new loaded_package();
			lp->beg = data;
			lp->end = data + data_sz;
			lp->should_free = false;
			lp->unresolved = 0;
			lp->unresolved_size = 0;
			lp->slots_size = slot_count;
			lp->slots = new package_slot[slot_count];
			
			pkg_ptrs _out_internal;
			pkg_ptrs &ptrs = out ? out->ptrs : _out_internal;
			
			for (int i=0;i!=slot_count;i++)
			{
				// path if wanted.
				uint16_t flags = parse_int16(&hdr_rp);
			
				if (flags & PKG_FLAG_PATH)
				{
					uint16_t path_len = parse_int16(&hdr_rp);
					lp->slots[i].path = strdup(hdr_rp);
					hdr_rp += path_len;
				}
				else
				{
					lp->slots[i].path = "<>";
				}
				
				lp->slots[i].flags = flags;
				
				if (flags & PKG_FLAG_EXTERNAL)
				{
					lp->slots[i].file_index = parse_int16(&hdr_rp);
					lp->slots[i].file_slot_index = parse_int16(&hdr_rp);
					lp->slots[i].type_id = parse_int16(&hdr_rp);
				}
				else if (flags & PKG_FLAG_INTERNAL)
				{
					lp->slots[i].file_index = -1;
					lp->slots[i].obj = data + (parse_int32(&hdr_rp) - hdr_sz);
					lp->slots[i].obj_end = data + (parse_int32(&hdr_rp) - hdr_sz);
					lp->slots[i].type_id = parse_int16(&hdr_rp);
				}
				else
				{
					if (!flags & PKG_FLAG_UNRESOLVED)
					{
						PTK_WARNING("Flag is not set to any valid combination, changing to unresolved: " << flags);
						lp->slots[i].flags = PKG_FLAG_UNRESOLVED;
					}

					lp->slots[i].file_index = -1;
					lp->slots[i].obj = 0;
					lp->slots[i].type_id = 0;
					
					lp->unresolved_size++;
				}
				
				PTK_DEBUG("Slot " << i << " path:" << lp->slots[i].path << " file:" << lp->slots[i].file_index << " obj:" << lp->slots[i].obj << " type:" << lp->slots[i].type_id);
			}
			
			// resolve objects
			for (unsigned int i=0;i!=slot_count;i++)
			{
				if (lp->slots[i].obj)
				{
					 if (post_load_by_type(lp->slots[i].type_id, &ptrs, (char*) lp->slots[i].obj, (char*) lp->slots[i].obj_end) != lp->slots[i].obj_end)
					 {
						PTK_WARNING("Post load by type (" << lp->slots[i].type_id << " did not consume all data.")
					 }
				}
					 
			}

			int resolved = 0, unresolved = 0;
			for (unsigned int i=0;i<ptrs.entries.size(); i++)
			{
				if (ptrs.entries[i].index > 0 && ptrs.entries[i].index <= (int)lp->slots_size)
				{
					package_slot *slot = &lp->slots[ptrs.entries[i].index-1];
					*(ptrs.entries[i].ptr) = slot->obj;
					
					if (slot->flags & PKG_FLAG_UNRESOLVED)
						unresolved++;
					else
						resolved++;
				}
				else
				{
					*(ptrs.entries[i].ptr) = 0;
				}
			}
		
			int op = 0;
			lp->unresolved = lp->unresolved_size ? (new const char*[lp->unresolved_size]) : 0;
			for (int i=0;i!=slot_count;i++)
			{
				if (lp->slots[i].flags & PKG_FLAG_UNRESOLVED)
				{
					lp->unresolved[op++] = strdup(lp->slots[i].path);
				}
			}
			
			if (op != lp->unresolved_size)
			{
				PTK_ERROR("Unresolved calculation malfunctionin");
			}
			
			if (unresolved++)
			{
				PTK_DEBUG("Package loaded with " << unresolved << " pointers.")
			}

			return lp;
		}

		void register_for_liveupdate(loaded_package *lp)
		{
			// maybe check here that there is nothing unresolved left. or we might start pointing into junk when stuff
			// start pointing into this.

			for (unsigned int i=0; i!=lp->slots_size; i++)
			{
				if (lp->slots[i].path) {
					putki::liveupdate::hookup_object(lp->slots[i].obj, lp->slots[i].path);
				}
			}
		}

		void release(loaded_package *lp)
		{
			if (lp->should_free) {
				delete [] lp->beg;
			}

			delete [] lp->unresolved;
			delete [] lp->slots;
			delete lp;
		}

		// -
		instance_t resolve(loaded_package *p, const char *path)
		{
			for (unsigned int i=0; i<p->slots_size; i++)
			{
				if (p->slots[i].path && !strcmp(p->slots[i].path, path)) {
					return p->slots[i].obj;
				}
			}
			return 0;
		}

		const char *path_in_package_slot(loaded_package *pkg, unsigned int slot)
		{
			if (slot < pkg->slots_size) {
				return pkg->slots[slot].path;
			}
			return 0;
		}

		const char *unresolved_reference(loaded_package *pkg, unsigned int index)
		{
			if (index < pkg->unresolved_size) {
				return pkg->unresolved[index];
			}
			return 0;
		}
	}

}