#include "liveupdate.h"

#include <map>

namespace putki
{
	namespace
	{
		typedef std::map<instance_t, instance_t> RewriteMap;
		RewriteMap s_rewrite;
	}

	namespace liveupdate
	{
		// returns true if updated, then pointer for new asset.
		bool update(instance_t *ptr)
		{
			RewriteMap::const_iterator i = s_rewrite.find(ptr);
			if (i != s_rewrite.end())
			{
				*ptr = i->second;
				return true;
			}

			return false;
		}
	}
}
