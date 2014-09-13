
namespace putki
{
	namespace runtime
	{
		inline const char *desc_str()
		{
			if (sizeof(void*) == 4)
				return "x32";
			else if (sizeof(void*) == 8)
				return "x64";
			else
				return "unknown";
		}
	}
}
