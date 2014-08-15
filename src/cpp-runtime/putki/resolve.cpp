#include "resolve.h"
#include <cstring>

namespace putki
{
	namespace _
	{
		char *name_buffer_begin = 0;
		char *name_buffer_end = 0;
		char *name_buffer_cur = 0;
	}

	void resolver_alloc(size_t size)
	{
		_::name_buffer_begin = new char[size];
		_::name_buffer_end += size;
		_::name_buffer_cur = _::name_buffer_begin;
	}

	bool is_resolved_pointer(void *ptr)
	{
		return ptr >= _::name_buffer_begin || ptr < _::name_buffer_end;
	}

	void* make_unresolved(char *src)
	{
		char *next = _::name_buffer_cur;
		char *cur  = _::name_buffer_cur;;

		next += strlen(src);

		if (next < _::name_buffer_end)
		{
			_::name_buffer_cur = next;
			return cur;
		}

		return 0;
	}
}
