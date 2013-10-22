
#include "pkgmgr.h"
#include "types.h"
#include "blob.h"
#include "liveupdate/liveupdate.h"

#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>

namespace putki
{
	namespace pkgmgr
	{
		struct package_slot
		{
			const char *path;
			instance_t obj;
			u32 type_id;
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
			for (unsigned int i=0;i<s->ptrs.entries.size();i++)
			{
				pkg_ptrs::entry &e = s->ptrs.entries[i];

				unsigned int path_index = (-e.index) - 1;
				if (path_index < target->unresolved_size)
				{
					std::cout << "Trying to resolve " << target->unresolved[path_index] << "!" << std::endl;

					instance_t nw = resolve(aux, target->unresolved[path_index]);
					if (nw)
					{
						std::cout << "Resolved!" << std::endl;
						resolved++;
						(*e.ptr) = nw;
					}
				}

				if (e.index < 0 && !*e.ptr)
					unresolved++;
			}

			return unresolved;
		}

		// parse from buffer
		loaded_package * parse(char *beg, char *end, resolve_status *out)
		{
			loaded_package *lp = new loaded_package;
			lp->beg = beg;
			lp->end = end;
			lp->should_free = false;
			lp->unresolved = 0;

			// slot entries.
			char *ptr = beg;
			lp->slots_size = *((u32*)ptr);
			lp->slots = new package_slot[lp->slots_size];

			ptr += 4;

			// store on the stack if nothing provided.
			pkg_ptrs _out_internal;
			pkg_ptrs &ptrs = out ? out->ptrs : _out_internal;

			for (unsigned int i=0;i!=lp->slots_size;i++)
			{
				const u32 has_path_flag = 1 << 31;
				const u32 type_id = (*((u32 *)ptr)) & (~has_path_flag);
				const bool has_path = (*((u32 *)ptr) & has_path_flag) != 0;

				ptr += 4;

				if (has_path)
				{
					const unsigned short pathlen = *((unsigned short *)ptr);
					lp->slots[i].path = ptr + 2;
					ptr += pathlen + 2;
				}
				else
				{
					lp->slots[i].path = "N/A";
				}

				lp->slots[i].obj = (instance_t) ptr;
				lp->slots[i].type_id = type_id;

				std::cout << "post_load_by_type(type=" << type_id << ") for path [" << lp->slots[i].path << "]" << std::endl;
				
				char *next = post_load_by_type(type_id, &ptrs, ptr, end);
				if (!next)
				{
					std::cout << "failed loading blob!" << std::endl;
					release(lp);
					return 0;
				}

				ptr = next;
			}

			lp->unresolved_size = (*((u32 *)ptr));
			lp->unresolved = new const char * [lp->unresolved_size];
			ptr += 4;
			for (unsigned int i=0;i!=lp->unresolved_size && ptr < end;i++)
			{
				const unsigned short pathlen = *((unsigned short *)ptr);
				lp->unresolved[i] = ptr + 2;
				ptr += pathlen + 2;
			}

			if (end == ptr)
			{
				int resolved = 0, unresolved = 0;
				for (unsigned int i=0;i<ptrs.entries.size();i++)
				{
					if (ptrs.entries[i].index > 0 && ptrs.entries[i].index <= (int)lp->slots_size)
					{
						*(ptrs.entries[i].ptr) = lp->slots[ptrs.entries[i].index-1].obj;
						resolved++;
					}
					else
					{
						*(ptrs.entries[i].ptr) = 0;
						if (ptrs.entries[i].index != 0)
							unresolved++;
					}
				}

				if (unresolved)
					std::cerr << " => Package loaded with unresolved ptrs! Count:" << unresolved << std::endl;
			}
			else
			{
				std::cerr << "Loaded with spare bytes! " << (end - ptr) <<  " bytes remaining." << std::endl;
			}

			return lp;
		}

		void register_for_liveupdate(loaded_package *lp)
		{
			// maybe check here that there is nothing unresolved left. or we might start pointing into junk when stuff
			// start pointing into this.
			for (unsigned int i=0;i!=lp->slots_size;i++)
			{
				if (lp->slots[i].path)
					putki::liveupdate::hookup_object(lp->slots[i].obj, lp->slots[i].path);
			}
		}

		void release(loaded_package *lp)
		{
			if (lp->should_free)
				delete [] lp->beg;

			delete [] lp->unresolved;
			delete [] lp->slots;
			delete lp;
		}

		// -
		instance_t resolve(loaded_package *p, const char *path)
		{
			for (unsigned int i=0;i<p->slots_size;i++)
			{
				if (p->slots[i].path && !strcmp(p->slots[i].path, path))
					return p->slots[i].obj;
			}
			return 0;
		}

		const char *path_in_package_slot(loaded_package *pkg, unsigned int slot)
		{
			if (slot < pkg->slots_size)
				return pkg->slots[slot].path;
			return 0;
		}

		const char *unresolved_reference(loaded_package *pkg, unsigned int index)
		{
			if (index < pkg->unresolved_size)
				return pkg->unresolved[index];
			return 0;
		}
	}

}