#include "treeparser.h"
#include "../buildconfigs.h"

#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <cstring>

#include <putki/sys/files.h>

namespace putki
{
	namespace
	{
		void do_file(const char *fullpath, const char *name, void *userptr)
		{
			project *proj = (project *) userptr;

			const char *ending = ".typedef";
			const unsigned int len = (unsigned int)strlen(ending);
			std::string fn(name);
			if (fn.size() <= len)
				return;

			if (fn.substr(fn.size() - len, len) == ending)
			{
				putki::parsed_file pf;
				pf.modulename = proj->module_name;
				if (putki::parse(fullpath, name, &pf))
				{
					proj->files.push_back(pf);
				}
				else
				{
					proj->parse_error = "Parse error!";
				}
			}
		}

		bool read_in_path(std::string base_path, project *out)
		{
			project tmp;

			std::string config_fn = base_path + "/putki-compiler.config";

			std::ifstream config(config_fn.c_str());
			if (!config.good())
			{
				std::cerr << config_fn << " could not be read." << std::endl;
				return false;
			}

			config >> tmp.module_name >> tmp.loader_name;

			const std::string dep_pfx("dep:");
			const std::string config_pfx("config:");

			std::string add;
			while (std::getline(config, add))
			{
				if (add.size() > dep_pfx.size() && add.substr(0, dep_pfx.size()) == dep_pfx)
				{
					std::string path = add.substr(dep_pfx.size(), add.size() - dep_pfx.size());
					tmp.deps.push_back(path);
				}
				if (add.size() > config_pfx.size() && add.substr(0, config_pfx.size()) == config_pfx)
				{
					std::string config = add.substr(config_pfx.size(), add.size() - config_pfx.size());
					tmp.build_configs.push_back(config.c_str());
					add_build_config(config.c_str());
				}
			}

			if (tmp.module_name.empty() || tmp.loader_name.empty())
			{
				std::cerr << config_fn << " does not specify module name and loader name " << std::endl;
				return false;
			}

			std::cout << config_fn << " loaded with " << tmp.deps.size() << " deps and " << tmp.build_configs.size() << " build configs" << std::endl;


			const std::string prefix = "/src";
			std::string inpath = base_path + prefix;

			tmp.start_path = base_path;
			tmp.base_path = inpath;

			putki::sys::search_tree(inpath.c_str(), do_file, &tmp);

			*out = tmp;
			return true;
		}
	}

	bool parse_all_with_deps(grand_parse *output)
	{
		std::set<std::string> visited;
		std::vector<std::string> next;

		next.push_back(".");

		while (!next.empty())
		{
			if (visited.count(next.back()))
			{
				next.pop_back();
				continue;
			}

			project tmp;
			if (!read_in_path(next.back().c_str(), &tmp))
				return false;

			output->projects.push_back(tmp);
			next.pop_back();

			for (int i=0;i!=tmp.deps.size();i++)
				next.push_back(tmp.deps[i]);
		}

		return true;
	}
}