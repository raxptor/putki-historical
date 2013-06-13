#ifndef __PUTKI_GENERATOR_H__
#define __PUTKI_GENERATOR_H__

#include <parser/parse.h>
#include <putki/runtime.h>
#include <generator/indentedwriter.h>

namespace putki
{
	void write_includes(putki::parsed_file *file, putki::indentedwriter out, bool inki = false);

	// if runtime descptr is null it means it's the real runtime, otherwise internal code for putki.
	void write_runtime_header(putki::parsed_file *file, runtime::descptr rt, putki::indentedwriter out);
	void write_runtime_impl(putki::parsed_file *file, runtime::descptr rt, putki::indentedwriter out);

	void write_runtime_blob_load_cases(putki::parsed_file *file, putki::indentedwriter out);
	
	void write_runtime_blob_load_cases(putki::parsed_file *file, putki::indentedwriter out);
	void write_runtime_blob_load_decl(const char *hpath, putki::indentedwriter out);
	
	void write_putki_header(putki::parsed_file *file, putki::indentedwriter out);
	void write_putki_impl(putki::parsed_file *file, putki::indentedwriter out);
 
	void write_bind_decl(putki::parsed_file *file, putki::indentedwriter out);
	void write_bind_calls(putki::parsed_file *file, putki::indentedwriter out);

	void write_dll_impl(putki::parsed_file *file, putki::indentedwriter out);
	void write_bind_decl_dll(putki::parsed_file *file, putki::indentedwriter out);
	void write_bind_call_dll(putki::parsed_file *file, putki::indentedwriter out);

}

#endif
