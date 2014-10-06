#ifndef __COMPILER_SAVE_STREAM_H__
#define __COMPILER_SAVE_STREAM_H__

#include <sstream>
#include <string>

namespace putki
{
	void save_stream(std::string path, std::stringstream &str);
}

#endif