#ifndef __GENERATOR_H__
#define __GENERATOR_H__

#include <parser/parse.h>
#include <putki/runtime.h>

namespace putki
{
	void write_input_header(putki::parsed_file *file);
	void write_input_impl(putki::parsed_file *file);

	void write_runtime_header(putki::parsed_file *file, putki::runtime rt);
	void write_runtime_impl(putki::parsed_file *file, putki::runtime rt);

	void write_meta_header(putki::parsed_file *file);
	void write_meta_impl(putki::parsed_file *file);
}

#endif
