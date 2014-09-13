#ifndef __PUTKI_RUNTIME_H__
#define __PUTKI_RUNTIME_H__

namespace putki
{
	namespace runtime
	{

		enum platform_t
		{
			PLATFORM_32BIT   = 0,
			PLATFORM_64BIT   = 1,
			PLATFORM_CSHARP  = 2,
			PLATFORM_UNKNOWN = 3
		};

		enum language_t
		{
			LANGUAGE_CPP = 0,
			LANGUAGE_CSHARP = 1
		};

		struct desc
		{
			platform_t platform;
			language_t language;
			int ptrsize;
			int boolsize;
			bool low_byte_first;
		};

		typedef const desc * descptr;

		const char *desc_str(desc const *rt);

		// enumerate
		const desc* get(unsigned int index);

		// current runtime
		const desc * running();

		inline int ptr_size(const desc *r) {
			return r->ptrsize;
		}
		inline bool has_low_byte_first(const desc *r) {
			return r->low_byte_first;
		}

		inline platform_t platform()
		{
			if (sizeof(void*) == 4)
				return PLATFORM_32BIT;
			if (sizeof(void*) == 8)
				return PLATFORM_64BIT;
			return PLATFORM_UNKNOWN;
		}
	}
}

#endif

