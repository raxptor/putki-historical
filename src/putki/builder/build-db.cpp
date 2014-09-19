#include "build-db.h"

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <set>
#include <fstream>
#include <cstring>

#include <putki/builder/db.h>
#include <putki/builder/log.h>

namespace putki
{
	namespace build_db
	{
		// metadata for -built- objects
		struct metadata
		{
			std::string type;
			std::string signature;
			std::set<std::string> pointers;
		};

		struct record
		{
			struct external_dep
			{
				std::string path;
				std::string signature;
			};

			std::string source_path;
			std::string source_sig;
			std::string builder;
			std::vector<std::string> input_dependencies;
			std::vector<external_dep> external_dependencies;
			std::vector<std::string> outputs;
			std::vector<std::string> builders;
			metadata md;
		};

		typedef std::map<std::string, record> RM;
		typedef std::multimap<std::string, std::string> RevDepMap;

		struct data
		{
			std::string path;
			RM records;
			RevDepMap depends;
		};

		data* create(const char *path, bool load)
		{
			data *d = new data();
			d->path = path;

			if (load)
			{
				std::ifstream dbtxt(d->path.c_str());
				if (dbtxt.good())
				{
					APP_DEBUG("Loading build db from [" << d->path << "]")
					record *cur = 0;
					std::string line;
					while (std::getline(dbtxt, line))
					{
						if (line.size() < 2) {
							continue;
						}

						std::string extra, extra2;

						// peel off extra2
						int w = line.find('*');
						if (w != std::string::npos)
						{
							extra2 = line.substr(w + 1, line.size() - w - 1);
							line.erase(w, line.size() - w);
						}

						// then extra
						w = line.find('@');
						if (w != std::string::npos)
						{
							extra = line.substr(w + 1, line.size() - w - 1);
							line.erase(w, line.size() - w);
						}

						const char *path = &line[2];

						if (line[0] == '#')
						{
							if (cur) {
								commit_record(d, cur);
							}
							cur = create_record(path, extra.c_str(), extra2.c_str());
						}
						else if (line[0] == 'i')
						{
							add_input_dependency(cur, path);
						}
						else if (line[0] == 'o')
						{
							add_output(cur, path, extra.c_str());
						}
						else if (line[0] == 'f')
						{
							add_external_resource_dependency(cur, path, extra.c_str());
						}
						else if (line[0] == 'p')
						{
							cur->md.pointers.insert(path);
						}
						else if (line[0] == 's')
						{
							cur->md.signature = path;
						}
						else if (line[0] == 't')
						{
							cur->md.type = path;
						}
						else
						{
							APP_WARNING("UNPARSED " << line)
						}
					}

					if (cur) {
						commit_record(d, cur);
					}
				}
			}

			return d;
		}

		void store(data *d)
		{
			std::ofstream dbtxt(d->path.c_str());
			APP_DEBUG("Writing build-db to [" << d->path << "]")

			for (RM::iterator i=d->records.begin(); i!=d->records.end(); i++)
			{
				record &r = i->second;

				// sources have extra argument signature, outputs have extra argument builder
				dbtxt << "#:" << i->first << "@" << r.source_sig << "*" << r.builder << "\n";

				for (unsigned int j=0; j!=r.input_dependencies.size(); j++)
					dbtxt << "i:" << r.input_dependencies[j] << "\n";
				for (unsigned int k=0; k!=r.external_dependencies.size(); k++)
					dbtxt << "f:" << r.external_dependencies[k].path << "@" << r.external_dependencies[k].signature << std::endl;
				for (unsigned int j=0; j!=r.outputs.size(); j++)
					dbtxt << "o:" << r.outputs[j] << "@" << r.builders[j] << "\n";

				dbtxt << "t:" << r.md.type << "\n";
				dbtxt << "s:" << r.md.signature << "\n";				

				std::set<std::string>::iterator pi = r.md.pointers.begin();
				while (pi != r.md.pointers.end())
					dbtxt << "p:" << (*pi++) << "\n";
			}

			if (!dbtxt.good())
			{
				APP_ERROR("Failed writing file!")
			}
		}

		void release(data *d)
		{
			delete d;
		}

		record *find(data *d, const char *output_path)
		{
			RM::iterator q = d->records.find(output_path);
			if (q != d->records.end())
				return &q->second;
			return 0;
		}

		const char *get_pointer(record *r, unsigned int index)
		{
			if (index >= r->md.pointers.size())
				return 0;

			std::set<std::string>::iterator it = r->md.pointers.begin();
			for (unsigned int i=1;i<index;i++)
				it++;

			return (*it).c_str();
		}

		const char *get_type(record *r)
		{
			return r->md.type.c_str();
		}

		const char *get_signature(record *r)
		{
			return r->md.signature.c_str();
		}

		record *create_record(const char *input_path, const char *input_signature, const char *builder)
		{
			record *r = new record();
			r->source_path = input_path;
			r->source_sig = input_signature;

			if (builder) {
				r->builder = builder;
			}
			return r;
		}

		void record_log(record *r, LogType type, const char *msg)
		{
			std::string pfx("[");
			pfx.append(r->source_path);
			pfx.append("]");
			print_log(type, pfx.c_str(), msg);
		}

		bool copy_existing(data *d, record *target, const char *path)
		{
			RM::iterator q = d->records.find(path);
			if (q != d->records.end())
			{
				*target = q->second;
				return true;
			}
			return false;
		}

		void set_builder(record *r, const char *builder)
		{
			r->builder = builder;
		}

		void add_output(record *r, const char *output_path, const char *builder)
		{
			// std::cout << "Adding output [" << output_path << "] [" << builder << "]" << std::endl;
			r->outputs.push_back(output_path);
			r->builders.push_back(builder);
		}

		void add_input_dependency(record *r, const char *dependency)
		{
			// aux filter.
			char tmp[1024];
			if (db::base_asset_path(dependency, tmp, sizeof(tmp)))
			{
				dependency = tmp;
			}

			// don't add same twice.
			for (unsigned int i=0; i<r->input_dependencies.size(); i++)
			{
				if (!strcmp(r->input_dependencies[i].c_str(), dependency)) 
				{
					return;
				}
			}

			r->input_dependencies.push_back(dependency);
		}

		void add_external_resource_dependency(record *r, const char *filepath, const char *signature)
		{
			for (int i=0; i<r->external_dependencies.size(); i++)
			{
				if (!strcmp(r->external_dependencies[i].path.c_str(), filepath))
				{
					r->external_dependencies[i].signature = signature;
					break;
				}
			}

			record::external_dep ed;
			ed.path = filepath;
			ed.signature = signature;
			r->external_dependencies.push_back(ed);
		}

		void copy_input_dependencies(record *copy_to, record *copy_from)
		{
			copy_to->input_dependencies = copy_from->input_dependencies;
		}

		void merge_input_dependencies(record *target, record *source)
		{
			for (int i=0; i<source->input_dependencies.size(); i++)
				add_input_dependency(target, source->input_dependencies[i].c_str());
			for (int i=0; i<source->external_dependencies.size(); i++)
				add_external_resource_dependency(target, source->external_dependencies[i].path.c_str(), source->external_dependencies[i].signature.c_str());
		}

		void append_extra_outputs(record *target, record *source)
		{
			for (unsigned int i=0; i<source->outputs.size(); i++)
			{
				if (source->outputs[i] != source->source_path)
				{
					target->outputs.push_back(source->outputs[i]);
					target->builders.push_back(source->builders[i]);
				}
			}
		}

		const char *enum_outputs(record *r, unsigned int pos)
		{
			if (pos < r->outputs.size()) {
				return r->outputs[pos].c_str();
			}
			return 0;
		}

		void cleanup_deps(data *d, record *r)
		{
			int count = 0;
			for (unsigned int i=0; i!=r->input_dependencies.size(); i++)
			{
				std::pair<RevDepMap::iterator, RevDepMap::iterator> range = d->depends.equal_range(r->input_dependencies[i]);
				for (RevDepMap::iterator j=range.first; j!=range.second; )
				{
					if (j->second == r->source_path)
					{
						// std::cout << "erasing " << j->first << " -> " << j->second << std::endl;
						count++;
						d->depends.erase(j++);
					}
					else
					{
						++j;
					}
				}
			}
			// std::cout << " -> Cleaned up " << count << " old dependencies" << std::endl;
		}

		void commit_record(data *d, record *r)
		{
			// std::cout << "Committing record " << r->source_path << std::endl;

			// clear up old if exists
			RM::iterator q = d->records.find(r->source_path);
			if (q != d->records.end())
			{
				cleanup_deps(d, &q->second);
				d->records.erase(q);
			}

			for (unsigned int i=0; i!=r->input_dependencies.size(); i++)
			{
				d->depends.insert(std::make_pair(r->input_dependencies[i], r->source_path));
				// std::cout << "Inserting extra record on " << r->input_dependencies[i] << " i am " << d << std::endl;
			}


			d->records.insert(std::make_pair(r->source_path, *r));
			delete r;
		}

		struct depwalker : putki::depwalker_i
		{
			db::data *db;
			metadata *out;

			virtual bool pointer_pre(instance_t * on)
			{
				if (!*on) {
					return true;
				}

				const char *path = db::pathof_including_unresolved(db, *on);
				if (!path)
				{
					APP_ERROR("Found object without path")
					return true;
				}

				// ignore aux paths since they are included implicitly.
				if (db::is_aux_path(path))
					return true;

				if (db::is_unresolved_pointer(db, *on))
				{
					out->pointers.insert(path);
					return false;
				}

				out->pointers.insert(path);
				return true;
			}

			void pointer_post(instance_t *on)
			{

			}
		};

		void insert_metadata(data *data, db::data *db, const char *path)
		{
			RM::iterator rec = data->records.find(path);
			if (rec == data->records.end())
			{
				APP_WARNING("No build record for " << path << ", fail to add metadata")
				return;
			}

			type_handler_i *th;
			instance_t obj;
			if (db::fetch(db, path, &th, &obj))
			{
				rec->second.md.type = th->name();
				rec->second.md.signature = db::signature(db, path);
				rec->second.md.pointers.clear();
				depwalker dw;
				dw.db = db;
				dw.out = &rec->second.md;
				th->walk_dependencies(obj, &dw, false);
			}
			else
			{
				APP_WARNING("Failed to fetch object for meta data insertion")
			}

		}

		struct deplist
		{
			struct entry
			{
				std::string path;
				std::string signature;
				std::string builder;
				bool is_external_resource; // file such as .png on disk
			};
			std::vector<entry> entries;
		};

		bool fill_entry(data *d, const char *path, deplist::entry *out)
		{
			out->path = path;
			out->is_external_resource = false;

			RM::iterator q = d->records.find(path);
			if (q != d->records.end())
			{
				out->signature = q->second.source_sig;
				out->builder = q->second.builder;
				return true;
			}

			// these will not have any records, only main objects.
			if (!db::is_aux_path(path))
			{
				APP_WARNING("OOPS! Could not find [" << path << "] in build record!")
			}

			return false;
		}

		deplist* deplist_get(data *d, const char *path)
		{
			deplist *dl = new deplist();
			std::pair<RevDepMap::iterator, RevDepMap::iterator> range = d->depends.equal_range(path);
			for (RevDepMap::iterator i=range.first; i!=range.second; i++)
			{
				deplist::entry e;
				if (!fill_entry(d, i->second.c_str(), &e)) {
					return 0;
				}

				dl->entries.push_back(e);
			}

			APP_DEBUG("Found " << dl->entries.size() << " dependant objects on [" << path << "]")
			return dl;
		}

		deplist* inputdeps_get(data *d, const char *path, bool paths_only)
		{
			deplist *dl = new deplist();

			RM::iterator q = d->records.find(path);
			if (q != d->records.end())
			{
				for (unsigned int i=0; i<q->second.input_dependencies.size(); i++)
				{
					deplist::entry e;
					if (paths_only)
					{
						e.path = q->second.input_dependencies[i];
						e.is_external_resource = false;
						dl->entries.push_back(e);

					}
					else
					{
						// fetch full
						if (fill_entry(d, q->second.input_dependencies[i].c_str(), &e)) {
							dl->entries.push_back(e);
						}
					}
				}
				for (unsigned int i=0; i<q->second.external_dependencies.size(); i++)
				{
					// file entry
					deplist::entry e;
					e.is_external_resource = true;
					e.path = q->second.external_dependencies[i].path;
					e.signature = q->second.external_dependencies[i].signature;
					dl->entries.push_back(e);
				}
			}

			return dl;
		}

		const char *deplist_entry(deplist *d, unsigned int index)
		{
			return deplist_path(d, index);
		}

		bool deplist_is_external_resource(deplist *d, unsigned int index)
		{
			if (index < d->entries.size()) {
				return d->entries[index].is_external_resource;
			}
			return false;
		}

		const char *deplist_path(deplist *d, unsigned int index)
		{
			if (index < d->entries.size()) {
				return d->entries[index].path.c_str();
			}
			return 0;
		}

		const char *deplist_signature(deplist *d, unsigned int index)
		{
			if (index < d->entries.size()) {
				return d->entries[index].signature.c_str();
			}
			return 0;
		}

		const char *deplist_builder(deplist *d, unsigned int index)
		{
			if (index < d->entries.size()) {
				return d->entries[index].builder.c_str();
			}
			return 0;
		}

		void deplist_free(deplist *d)
		{
			delete d;
		}
	}
}