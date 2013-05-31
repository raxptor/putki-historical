#include "write.h"

#include <iostream>
#include <sstream>
#include <iomanip>

namespace putki
{
	namespace write
	{
		void write_object_into_stream(std::ostream& out, db::data *ref_source, type_handler_i *th, instance_t obj)
		{
			out << "{" << std::endl;
			out << "	type: " << json_str(th->name()) << "," << std::endl;
			out << "	data: {\n";
			th->write_json(ref_source, obj, out, 2);
			out << "	}" << std::endl;
			out << "}" << std::endl;
		}

		std::string json_str(const char *input)
		{
			std::string s(input);
			std::stringstream ss;
			ss << "\"";
			for (size_t i = 0; i < s.length(); ++i) {
				if (unsigned(s[i]) < '\x20' || s[i] == '\\' || s[i] == '"') {
					ss << "\\u" << std::setfill('0') << std::setw(4) << std::hex << unsigned(s[i]);
				} else {
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
			for (i=0;i<level;i++)
				buf[i] = '\t';
			buf[i] = 0;
			return buf;
		}
	}
}

