
namespace putki
{
	namespace runtime
	{
		inline const char *desc_str()
		{
			#if defined(_WIN32)
				if (sizeof(void*) == 4)
					return "win32";
				else
					return "win64";
			#elif defined(__APPLE__)
				if (sizeof(void*) == 4)
					return "macosx32";
				else
					return "macosx64";
			#else
				return "unknown";
			#endif
		}
	}
}
