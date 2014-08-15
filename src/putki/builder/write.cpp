#include "write.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>

namespace putki
{
	namespace write
	{
		struct auxwriter : public depwalker_i
		{
			std::vector<std::string> paths;
			std::vector<std::string> subpaths;
			db::data *ref_source;
			instance_t base;
			type_handler_i *th;

			virtual bool pointer_pre(instance_t *on)
			{
				instance_t obj = *on;
				if (!obj) {
					return false;
				}

				const char *path = db::pathof(ref_source, obj);
				if (path && db::is_aux_path_of(ref_source, base, path))
				{
					// std::cout << "Including aux object [" << path << "]" << std::endl;
					paths.push_back(path);

					auxwriter aw;
					if (db::fetch(ref_source, path, &aw.th, &aw.base))
					{
						aw.ref_source = ref_source;
						aw.th->walk_dependencies(aw.base, &aw, false);

						for (unsigned int i=0; i<aw.paths.size(); i++)
							subpaths.push_back(aw.paths[i]);
						for (unsigned int i=0; i<aw.subpaths.size(); i++)
							subpaths.push_back(aw.subpaths[i]);
					}
					else
					{
						*on = 0;
					}
				}

				return true;
			}

			virtual void pointer_post(instance_t *on)
			{

			}
		};


		void write_object_into_stream(std::ostream& out, db::data *ref_source, type_handler_i *th, instance_t obj)
		{
			out << "{" << std::endl;
			out << "	type: "<< json_str(th->name()) << "," << std::endl;
			out << "	data: {\n";
			th->write_json(ref_source, obj, out, 1);
			out << "	},"<< std::endl;

			// collect all aux objects.
			std::vector<std::string> paths;

			auxwriter aw;
			aw.th = th;
			aw.base = obj;
			aw.ref_source = ref_source;
			th->walk_dependencies(obj, &aw, false);

			// merge
			for (unsigned int i=0; i<aw.subpaths.size(); i++)
				aw.paths.push_back(aw.subpaths[i]);

			out << "	aux: ["<< std::endl;
			for (unsigned int i=0; i<aw.paths.size(); i++)
			{
				if (i > 0) {
					out << "		,"<< std::endl;
				}

				type_handler_i *th;
				instance_t obj;
				db::fetch(ref_source, aw.paths[i].c_str(), &th, &obj);

				int sp = aw.paths[i].find_first_of('#');

				out << "		{"<< std::endl;
				out << "			ref: \""<< aw.paths[i].substr(sp, aw.paths[i].size() - sp) << "\"," << std::endl;
				out << "			type: "<< json_str(th->name()) << "," << std::endl;
				out << "			data: {\n";
				th->write_json(ref_source, obj, out, 4);
				out << "			}"<< std::endl;
				out << "		}"<< std::endl;
			}

			out << "	]"<< std::endl;

			// now all the aux

			out << "}" << std::endl;
		}

		std::string json_str(const char *input)
		{
			if (!input) {
				return "\"\"";
			}

			std::string s(input);
			std::stringstream ss;
			ss << "\"";
			for (size_t i = 0; i < s.length(); ++i) {
				if (unsigned(s[i]) < '\x20' || s[i] == '\\' || s[i] == '"') {
					ss << "\\u" << std::setfill('0') << std::setw(4) << std::hex << unsigned(s[i]);
				}
				else{
					ss << s[i];
				}
			}
			ss << "\"";
			return ss.str();
		}

		const char *json_indent(int level)
		{
			static char buf[256];
			int i;
			for (i=0; i<level; i++)
				buf[i] = '\t';
			buf[i] = 0;
			return buf;
		}
	}
}

