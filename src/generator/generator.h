#ifndef __PUTKI_GENERATOR_H__
#define __PUTKI_GENERATOR_H__

#include <parser/parse.h>
#include <putki/runtime.h>
#include <iostream>

namespace putki
{
	void write_runtime_header(putki::parsed_file *file, putki::runtime rt, std::ostream &out);
	void write_runtime_impl(putki::parsed_file *file, putki::runtime rt, std::ostream &out);

	void write_putki_header(putki::parsed_file *file, std::ostream &out);
	void write_putki_impl(putki::parsed_file *file, std::ostream &out);
    
  	void write_bind_calls(putki::parsed_file *file, std::ostream &out);
}

#endif
