#ifndef __PUTKI_BRIDGE_H__
#define __PUTKI_BRIDGE_H__

#include <string>
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::IO;

namespace putki
{

	struct ext_type_handler_i;

	public ref class TypeDefinition
	{
		public:
			TypeDefinition(const ext_type_handler_i *handler);
			~TypeDefinition();

			String^ GetName();

		private:

			const ext_type_handler_i * handler;
	};

	public ref class Sys
	{
		public:

			Sys();
			~Sys();

			void load(String^ dll);

			TypeDefinition^ get_type_definition(String^ str);
			TypeDefinition^ get_type_by_index(int i);
			
		private:
	};


}

#endif
