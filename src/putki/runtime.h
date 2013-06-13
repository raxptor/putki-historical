#ifndef __PUTKI_RUNTIME_H__
#define __PUTKI_RUNTIME_H__

namespace putki
{
	namespace runtime
	{

		enum platform_t
		{
			PLATFORM_WINDOWS = 0,
			PLATFORM_MACOSX = 1,
			PLATFORM_UNKNOWN,
		};

		struct desc
		{
			platform_t platform;
			int ptrsize;
			bool low_byte_first;
		};

		typedef const desc * descptr;

		const char *desc_str(desc const *rt);
	
		// enumerate
		const desc* get(unsigned int index);

		// current runtime
		const desc * running();

		inline int ptr_size(const desc *r) { return r->ptrsize; }
		inline bool has_low_byte_first(const desc *r) { return r->low_byte_first; }

		inline platform_t platform()
		{
			#if defined(_WIN32)
					return PLATFORM_WINDOWS;
			#elif defined(__APPLE__)
					return PLATFORM_MACOSX;
			#else
					return PLATFORM_UNKNOWN;
			#endif
		}
	}
}

#endif

