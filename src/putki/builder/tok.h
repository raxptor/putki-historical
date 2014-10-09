#ifndef __PUTKI_TOK_H__
#define __PUTKI_TOK_H__

// quick tokenizatino

namespace putki
{
	namespace tok
	{
		struct data;
		
		data* load(const char *fn);
		void free(data *d);
		
		void tokenize_newlines(data *d);
		
		int size(data *d);
		const char *get(data *d, unsigned int index);
	}
}

#endif
