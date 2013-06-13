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

	public enum FieldType
	{
		EXT_FIELDTYPE_INT32 = 0,
		EXT_FIELDTYPE_BYTE = 1,
		EXT_FIELDTYPE_STRING = 2,
		EXT_FIELDTYPE_POINTER = 3,
		EXT_FIELDTYPE_STRUCT_INSTANCE = 4,
		EXT_FIELDTYPE_FILE = 5,
		EXT_FIELDTYPE_INVALID = 6
	};

	public ref class TypeDefinition
	{
		public:
			TypeDefinition(putki::ext_type_handler_i *handler);
			~TypeDefinition();

			String^ GetName();

			TypeDefinition^ GetParentType();

			FieldHandler^ GetField(int i);
			putki::ext_type_handler_i * GetPutkiTypeDefinition() { return handler; }

		private:

			putki::ext_type_handler_i * handler;
	};

	public ref class MemInstance
	{
		public:
			MemInstance(TypeDefinition^ type, putki::mem_instance *mem_instance);
			~MemInstance();

			TypeDefinition^ GetType() { return m_type; }
			String^ GetPath();

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

			FieldType GetType();
			TypeDefinition^ GetRefType();
			MemInstance^ GetStructInstance(MemInstance^ Obj);

			bool ShowInEditor();

			String^ GetName();
			bool IsArray();
			int GetArraySize(MemInstance^ instance);
			void SetArrayIndex(int index);
			void ArrayInsert(MemInstance^ instance);
			void ArrayErase(MemInstance^ instance);

			String^ GetString(MemInstance^ instance);
			void SetString(MemInstance^ instance, String^ Value);

			int GetInt32(MemInstance^ instance);
			void SetInt32(MemInstance^ instance, int Value);

			bool GetBool(MemInstance^ instance);
			void SetBool(MemInstance^ instance, bool Value);

			float GetFloat(MemInstance^ instance);
			void SetFloat(MemInstance^ instance, float Value);

			String^ GetPointer(MemInstance^ instance);
			void SetPointer(MemInstance^ instance, String^ Value);
			bool IsAuxPtr();

			int GetByte(MemInstance^ instance);
			void SetByte(MemInstance^ instance, int Value);

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
			static void MemBuildAsset(String^ path);
			static MemInstance^ CreateAuxInstance(MemInstance ^onto, TypeDefinition^ type);
	};

};

#endif
