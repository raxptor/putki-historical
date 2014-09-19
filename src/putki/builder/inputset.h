#ifndef __PUTKI_INPUTSET_H__
#define __PUTKI_INPUTSET_H__

namespace putki
{
	namespace inputset
	{
		struct data;
		
		data *open(const char *objpath, const char *respath, const char *dbfile);
		void force_obj(data *d, const char *objpath, const char *signature);
		void write(data *d);
		void release(data *);
		
		const char *get_object_sig(data *d, const char *path);
	}
}

#endif
