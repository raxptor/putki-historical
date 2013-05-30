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
	struct mem_instance;
}

namespace Putki
{

	public ref class FieldHandler
	{
		public:
			FieldHandler(const char *name)
			{
				m_name = gcnew String(name);
			}

			String^ GetName() { return m_name; }

		private:

			String^ m_name;
	};

	public ref class TypeDefinition
	{
		public:
			TypeDefinition(putki::ext_type_handler_i *handler);
			~TypeDefinition();

			String^ GetName();

			FieldHandler^ GetField(int i);

		private:

			putki::ext_type_handler_i * handler;
	};

	public ref class MemInstance
{	jri
		public:
			MemInstance(TypeDefinition^ type, putki::mem_instance *mem_instance);
			~MemInstance();

			TypeDefinition^ GetType() { return m_type; }

		private:

			TypeDefinition^ m_type;
			putki::mem_instance *m_instance;
	};

	public ref class Sys
	{
		public:
			static void Load(String^ dll, String^ datapath);
			static MemInstance^ LoadFromDisk(String^ path);
			static TypeDefinition^ GetTypeByIndex(int i);
	};

};

#endif
