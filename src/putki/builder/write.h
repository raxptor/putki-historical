#ifndef __PUTKI_WRITER_H__
#define __PUTKI_WRITER_H__

#include <putki/builder/typereg.h>
#include <putki/builder/db.h>
#include <putki/sys/sstream.h>
#include <vector>

namespace putki
{
	namespace db { struct data; }

	namespace write
	{
		void write_object_into_stream(putki::sstream & out, db::data *ref_source, type_handler_i *th, instance_t obj);
		std::string json_str(const char *input);
		const char *json_indent(char *buf, int level);
		void json_hexstream(putki::sstream & out, std::vector<unsigned char> const &bytes);
		
		bool write_object_to_fs(const char *basedir, const char *path, db::data *ref_source, type_handler_i *th, instance_t obj, char *fn_out);
	}
}

#endif
