#ifndef __PUTKI_INPUTSET_H__
#define __PUTKI_INPUTSET_H__

namespace putki
{
	namespace inputset
	{
		struct data;
		
		struct manifest_record
		{
			char signature[64];
		};
		
		data *open(const char *objpath, const char *respath, const char *dbfile);
		void release(data *);
		
		void on_changed(const char *path);
		bool get_record(const char *path, manifest_record *out);
	}
}

#endif
