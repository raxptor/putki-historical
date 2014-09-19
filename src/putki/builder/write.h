#ifndef __PUTKI_WRITER_H__
#define __PUTKI_WRITER_H__

#include <putki/builder/typereg.h>
#include <putki/builder/db.h>
#include <string>

namespace putki
{
	namespace write
	{
		void write_object_into_stream(std::ostream & out, db::data *ref_source, type_handler_i *th, instance_t obj);
		std::string json_str(const char *input);
		const char *json_indent(int level);
		
		bool write_object_to_fs(const char *basedir, const char *path, db::data *ref_source, type_handler_i *th, instance_t obj, char *fn_out);
	}
}

#endif
