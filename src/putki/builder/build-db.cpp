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
#include <putki/sys/thread.h>

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
		
		typedef std::pair<LogType, std::string> logentry_t;

		struct record
		{
			struct dep_entry
			{
				std::string path;
				std::string signature;
			};

			std::string source_path;
			std::string source_sig;
			std::string builder;
			std::string parent_object;
			std::vector<dep_entry> input_dependencies;
			std::vector<dep_entry> dependencies;
			std::vector<std::string> outputs;
			std::vector<std::string> builders;
			
			std::vector<logentry_t> logs;
			metadata md;
		};

		typedef std::map<std::string, record*> RM;
		typedef std::multimap<std::string, std::string> RevDepMap;

		struct data
		{
			std::string path;
			RM records;
			RevDepMap depends;
			sys::mutex mtx;
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
							add_input_dependency(cur, path, extra.c_str());
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
						else if (line[0] == 'c')
						{
							cur->parent_object = path;
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
				record &r = *(i->second);

				// sources have extra argument signature, outputs have extra argument builder
				dbtxt << "#:" << i->first << "@" << r.source_sig << "*" << r.builder << "\n";

				if (!r.parent_object.empty())
					dbtxt << "c:" << r.parent_object << "\n";

				for (unsigned int j=0; j!=r.input_dependencies.size(); j++)
				{
					// update source signature here
					RM::iterator q = d->records.find(r.input_dependencies[j].path);
					if (q != d->records.end())
						r.input_dependencies[j].signature = q->second->source_sig;
					else
						APP_ERROR("Could not find build entry for sig update " << r.input_dependencies[j].path)
						
					dbtxt << "i:" << r.input_dependencies[j].path << "@" << r.input_dependencies[j].signature;
					dbtxt << "\n";
				}
				for (unsigned int k=0; k!=r.dependencies.size(); k++)
					dbtxt << "f:" << r.dependencies[k].path << "@" << r.dependencies[k].signature << std::endl;
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
			RM::iterator q = d->records.begin();
			while (q != d->records.end())
				delete ((q++)->second);
			delete d;
		}

		record *find(data *d, const char *output_path)
		{
			sys::scoped_maybe_lock _lk(&d->mtx);
			RM::iterator q = d->records.find(output_path);
			if (q != d->records.end())
				return q->second;
			return 0;
		}

		const char *get_pointer(record *r, unsigned int index)
		{
			if (index >= r->md.pointers.size())
				return 0;

			std::set<std::string>::iterator it = r->md.pointers.begin();
			for (unsigned int i=0;i<index;i++)
				it++;

			return (*it).c_str();
		}
		
		const char *get_builder(record *r) { return r->builder.c_str(); }
		const char *get_type(record *r) { return r->md.type.c_str(); }
		const char *get_signature(record *r) { return r->md.signature.c_str(); }
		const char *get_parent(record *r) { return r->parent_object.empty() ? 0 : r->parent_object.c_str(); }

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
		
		void flush_log(record *r)
		{
			std::string pfx("[");
			pfx.append(r->source_path);
			pfx.append("]");
		
			while (!r->logs.empty())
			{
				const unsigned int max = 64;
				LogType types[max];
				const char *messages[max];
				
				unsigned int count = 0;
				while (count < max && count < r->logs.size())
				{
					types[count] = r->logs[count].first;
					messages[count] = r->logs[count].second.c_str();
					count++;
				}
				
				print_log_multi(pfx.c_str(), types, messages, count);
				r->logs.erase(r->logs.begin(), r->logs.begin() + count);
			}
		}

		void record_log(record *r, LogType type, const char *msg)
		{
			r->logs.push_back(logentry_t(type, msg));
			// if (type == LOG_ERROR)
			{
				flush_log(r);
			}
		}

		bool copy_existing(data *d, record *target, const char *path)
		{
			sys::scoped_maybe_lock _lk(&d->mtx);
			RM::iterator q = d->records.find(path);
			if (q != d->records.end())
			{
				std::vector<logentry_t> logs = target->logs;
				*target = *(q->second);
				target->logs.insert(target->logs.begin(), logs.begin(), logs.end());
				return true;
			}
			return false;
		}

		void set_parent(record *r, const char *parent)
		{
			r->parent_object = parent;
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

		void add_input_dependency(record *r, const char *dependency, const char *signature)
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
				if (!strcmp(r->input_dependencies[i].path.c_str(), dependency)) 
				{
					if (signature)
						r->input_dependencies[i].signature = signature;
					return;
				}
			}
				
			record::dep_entry de;
			de.path = dependency;
			de.signature = (signature ? signature : "");
			r->input_dependencies.push_back(de);
		}

		void add_external_resource_dependency(record *r, const char *filepath, const char *signature)
		{
			for (int i=0; i<r->dependencies.size(); i++)
			{
				if (!strcmp(r->dependencies[i].path.c_str(), filepath))
				{
					r->dependencies[i].signature = signature;
					break;
				}
			}

			record::dep_entry ed;
			ed.path = filepath;
			ed.signature = signature;
			r->dependencies.push_back(ed);
		}

		void copy_input_dependencies(record *copy_to, record *copy_from)
		{
			copy_to->input_dependencies = copy_from->input_dependencies;
		}

		void merge_input_dependencies(record *target, record *source)
		{
			for (int i=0; i<source->input_dependencies.size(); i++)
				add_input_dependency(target, source->input_dependencies[i].path.c_str(), source->input_dependencies[i].signature.c_str());
			for (int i=0; i<source->dependencies.size(); i++)
				add_external_resource_dependency(target, source->dependencies[i].path.c_str(), source->dependencies[i].signature.c_str());
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
				std::pair<RevDepMap::iterator, RevDepMap::iterator> range = d->depends.equal_range(r->input_dependencies[i].path);
				for (RevDepMap::iterator j=range.first; j!=range.second; )
				{
					if (j->second == r->source_path)
					{
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
			sys::scoped_maybe_lock _lk(&d->mtx);

			// clear up old if exists
			RM::iterator q = d->records.find(r->source_path);
			if (q != d->records.end())
			{
				cleanup_deps(d, q->second);
				delete q->second;
				d->records.erase(q);
			}

			for (unsigned int i=0; i!=r->input_dependencies.size(); i++)
			{
				d->depends.insert(std::make_pair(r->input_dependencies[i].path, r->source_path));
				// std::cout << "Inserting extra record on " << r->input_dependencies[i] << " i am " << d << std::endl;
			}

			flush_log(r);

			d->records.insert(std::make_pair(r->source_path, r));
		}

		struct depwalker : putki::depwalker_i
		{
			db::data *db;
			metadata *out;

			virtual bool pointer_pre(instance_t * on, const char *ptr_type)
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
				{
					out->pointers.insert(path);
					return true;
				}

				if (db::is_unresolved_pointer(db, *on))
				{
					out->pointers.insert(path);
					return false;
				}

				out->pointers.insert(path);
				return false;
			}

			void pointer_post(instance_t *on)
			{

			}
		};

		void insert_metadata(data *data, db::data *db, const char *path)
		{
			data->mtx.lock();
			RM::iterator rec = data->records.find(path);
			if (rec == data->records.end())
			{
				APP_WARNING("No build record for " << path << ", fail to add metadata")
				data->mtx.unlock();
				return;
			}
			data->mtx.unlock();

			type_handler_i *th;
			instance_t obj;
			if (db::fetch(db, path, &th, &obj))
			{
				char buffer[128];
				rec->second->md.type = th->name();
				rec->second->md.signature = db::signature(db, path, buffer);
				rec->second->md.pointers.clear();
				depwalker dw;
				dw.db = db;
				dw.out = &rec->second->md;
				th->walk_dependencies(obj, &dw, true);
			}
			else
			{
				APP_WARNING("Failed to fetch [" << path << "] for meta data insertion")
			}

		}

		struct deplist
		{
			struct entry
			{
				std::string path;
				std::string signature;
				bool is_external_resource; // file such as .png on disk
			};
			std::vector<entry> entries;
		};

		deplist* deplist_get(data *d, const char *path)
		{
			sys::scoped_maybe_lock lk(&d->mtx);
		
			deplist *dl = new deplist();
			std::pair<RevDepMap::iterator, RevDepMap::iterator> range = d->depends.equal_range(path);
			for (RevDepMap::iterator i=range.first; i!=range.second; i++)
			{
				deplist::entry e;
				e.path = i->second;
				e.is_external_resource = false;
				dl->entries.push_back(e);
			}

			APP_DEBUG("Found " << dl->entries.size() << " dependant objects on [" << path << "]")
			return dl;
		}

		deplist* inputdeps_get(data *d, const char *path)
		{
			sys::scoped_maybe_lock lk(&d->mtx);
		
			deplist *dl = new deplist();

			RM::iterator q = d->records.find(path);
			if (q != d->records.end())
			{
				for (unsigned int i=0; i<q->second->input_dependencies.size(); i++)
				{
					deplist::entry e;
					e.path = q->second->input_dependencies[i].path;
					e.signature = q->second->input_dependencies[i].signature;
					e.is_external_resource = false;
					dl->entries.push_back(e);
				}
				for (unsigned int i=0; i<q->second->dependencies.size(); i++)
				{
					// file entry
					deplist::entry e;
					e.is_external_resource = true;
					e.path = q->second->dependencies[i].path;
					e.signature = q->second->dependencies[i].signature;
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

		void deplist_free(deplist *d)
		{
			delete d;
		}
	}
}