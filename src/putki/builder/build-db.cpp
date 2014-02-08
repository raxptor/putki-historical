#include "build-db.h"

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <fstream>

namespace putki
{
	namespace build_db
	{
		struct record
		{
			std::string source_path;
			std::string source_sig;
			std::string builder;
			std::vector<std::string> input_dependencies;
			std::vector<std::string> outputs;
			std::vector<std::string> builders;
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
					std::cout << "Replaying build db from [" << d->path << "]" << std::endl;
					record *cur = 0;
					std::string line;
					while (std::getline(dbtxt, line))
					{
						if (line.size() < 2)
							continue;
						
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
							if (cur)
								commit_record(d, cur);
							cur = create_record(path, extra.c_str(), extra2.c_str());
						}
						else if (line[0] == 'i')
						{
							add_input_dependency(cur, path);
						}
						else if (line[0] == 'o')
						{
							int w = line.find('@');
							if (w != std::string::npos)
							{
								add_output(cur, path, extra.c_str());
							}
						}
					}
				}
			}
			
			return d;
		}
		
		void store(data *d)
		{
			std::ofstream dbtxt(d->path.c_str());
			std::cout << "Writing build-db to [" << d->path << "]" << std::endl;
			for (RM::iterator i=d->records.begin();i!=d->records.end();i++)
			{
				record &r = i->second;
	
				// sources have extra argument signature, outputs have extra argument builder
				dbtxt << "#:" << i->first << "@" << r.source_sig << "*" << r.builder << "\n";

				for (unsigned int j=0;j!=r.input_dependencies.size();j++)
					dbtxt << "i:" << r.input_dependencies[j] << "\n";
				for (unsigned int j=0;j!=r.outputs.size();j++)
					dbtxt << "o:" << r.outputs[j] << "@" << r.builders[j] << "\n";
			}
		}

		void release(data *d)
		{
			delete d;
		}

		record *create_record(const char *input_path, const char *input_signature, const char *builder)
		{
			record *r = new record();
			r->source_path = input_path;
			r->source_sig = input_signature;
			
			if (builder)
				r->builder = builder;
			return r;
		}
		
		void set_builder(record *r, const char *builder)
		{
			r->builder = builder;
		}

		void add_output(record *r, const char *output_path, const char *builder)
		{
			std::cout << "Adding output [" << output_path << "] [" << builder << "]" << std::endl;
			r->outputs.push_back(output_path);
			r->builders.push_back(builder);
		}

		void add_input_dependency(record *r, const char *dependency)
		{
			r->input_dependencies.push_back(dependency);
		}

		void copy_input_dependencies(record *copy_to, record *copy_from)
		{
			copy_to->input_dependencies = copy_from->input_dependencies;
		}

		void append_extra_outputs(record *target, record *source)
		{
			for (unsigned int i=0;i<source->outputs.size();i++)
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
			if (pos < r->outputs.size())
				return r->outputs[pos].c_str();
			return 0;
		}
		
		void cleanup_deps(data *d, record *r)
		{
			int count = 0;
			for (unsigned int i=0;i!=r->input_dependencies.size();i++)
			{
				std::pair<RevDepMap::iterator, RevDepMap::iterator> range = d->depends.equal_range(r->input_dependencies[i]);
				for (RevDepMap::iterator j=range.first;j!=range.second;)
				{
					if (j->second == r->source_path)
					{
						std::cout << "erasing " << j->first << " -> " << j->second << std::endl;
						count++;
						d->depends.erase(j++);
					}
					else
					{
						++j;
					}
				}
			}
			std::cout << " -> Cleaned up " << count << " old dependencies" << std::endl;
		}

		void commit_record(data *d, record *r)
		{
			std::cout << "Committing record " << r->source_path << std::endl;
			
			// clear up old if exists
			RM::iterator q = d->records.find(r->source_path);
			if (q != d->records.end())
			{
				cleanup_deps(d, &q->second);
			}

			for (unsigned int i=0;i!=r->input_dependencies.size();i++)
			{
				d->depends.insert(std::make_pair(r->input_dependencies[i], r->source_path));
				std::cout << "Inserting extra record on " << r->input_dependencies[i] << " i am " << d << std::endl;
			}
			
			d->records.insert(std::make_pair(r->source_path, *r));
			delete r;
		}
		
		struct deplist
		{
			std::vector<std::string> entries;
		};
		
		deplist* deplist_get(data *d, const char *path)
		{
			deplist *dl = new deplist();
			std::pair<RevDepMap::iterator, RevDepMap::iterator> range = d->depends.equal_range(path);
			for (RevDepMap::iterator i=range.first;i!=range.second;i++)
				dl->entries.push_back(i->second);
				
			std::cout << "Found " << dl->entries.size() << " dependant objects on [" << path << "]" << std::endl;
			return dl;
		}
		
		const char *deplist_entry(deplist *d, unsigned int index)
		{
			if (index < d->entries.size())
				return d->entries[index].c_str();
			return 0;
		}
		
		void deplist_free(deplist *d)
		{
			delete d;
		}
	}
}