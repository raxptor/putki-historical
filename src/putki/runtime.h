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
			PLATFORM_CSHARP = 2,
			PLATFORM_UNIXY = 3,
			PLATFORM_UNKNOWN,
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
			#if defined(_WIN32)
			return PLATFORM_WINDOWS;
			#elif defined(__APPLE__)
<<<<<<< HEAD
			return PLATFORM_MACOSX;
			#elif defined(linux)
			return PLATFORM_LINUX;
=======
					return PLATFORM_MACOSX;
			#elif defined(linux) || defined(BSD) || defined(__FreeBSD__)
					return PLATFORM_UNIXY;
>>>>>>> 7182c4a... - Merged platforms to UNIXY (for freebsd,linux now)
			#else
					#error Add runtime definition for your platform, or fix the ifdefs here!
			return PLATFORM_UNKNOWN;
			#endif
		}
	}
}

#endif

