#ifndef __PUTKI_INPUTSET_H__
#define __PUTKI_INPUTSET_H__

namespace putki
{
	namespace inputset
	{
		struct data;
		
		data *open(const char *objpath, const char *respath, const char *dbfile);
		void release(data *);
		
		const char *get_object_sig(data *d, const char *path);
	}
}

#endif
