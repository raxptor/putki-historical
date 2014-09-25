#ifndef __PUTKI_INPUTSET_H__
#define __PUTKI_INPUTSET_H__

namespace putki
{
	namespace inputset
	{
		struct data;
		
		data *open(const char *objpath, const char *respath, const char *dbfile);
		void force_obj(data *d, const char *objpath, const char *signature, const char *type);
		void touched_resource(data *d, const char *path);

		void write(data *d);
		void release(data *);
		
		#define SIG_BUF_SIZE 64
		bool get_object_sig(data *d, const char *path, char *buffer);
		const char *get_object_type(data *d, const char *path);
		bool get_res_sig(data *d, const char *path, char *buffer);
	}
}

#endif
