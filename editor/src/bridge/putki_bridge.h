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
	struct ext_field_handler_i;
	struct mem_instance;
}

namespace Putki
{

	ref class FieldHandler;

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
	{
		public:
			MemInstance(TypeDefinition^ type, putki::mem_instance *mem_instance);
			~MemInstance();

			TypeDefinition^ GetType() { return m_type; }

			putki::mem_instance* GetPutkiMemInstance() { return m_instance; }

		private:

			TypeDefinition^ m_type;
			putki::mem_instance *m_instance;
	};

	public ref class FieldHandler
	{
		public:
			FieldHandler(putki::ext_field_handler_i *handler)
			{
				m_handler = handler;
			}

			String^ GetName();
			String^ GetString(MemInstance^ instance);
			void SetString(MemInstance^ instance, String^ Value);

		private:

			putki::ext_field_handler_i *m_handler;
	};


	public ref class Sys
	{
		public:
			static void Load(String^ dll, String^ datapath);
			static MemInstance^ LoadFromDisk(String^ path);
			static void SaveObject(MemInstance ^mi);
			static TypeDefinition^ GetTypeByIndex(int i);
	};

};

#endif
