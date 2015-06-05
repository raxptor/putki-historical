package putked;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.HashMap;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;

public class Interop 
{
	public interface NI extends Library
	{
		public void MED_Initialize(String dllPath, String dataPath);

		public Pointer MED_TypeByIndex(int i);
		public Pointer MED_TypeByName(String n);
		
		// mem objects
		public Pointer MED_DiskLoad(String path);
		public Pointer MED_TypeOf(Pointer obj);
		public String MED_PathOf(Pointer obj);
		public void MED_DiskSave(Pointer p);
		public void MED_OnObjectModified(Pointer p);
		
		public String MED_MakeJSON(Pointer obj);		
		public String MED_ContentHash(Pointer obj);
		
		// types
		public String MED_Type_GetName(Pointer p);
		public String MED_Type_GetModuleName(Pointer p);	
		public Pointer MED_Type_GetField(Pointer type, int index);
		public Pointer MED_Type_GetParentType(Pointer type);
		int MED_Type_PermitAsAsset(Pointer type);
		int MED_Type_PermitAsAuxInstance(Pointer type);
		
		public Pointer MED_CreateInstance(String path, Pointer type);

		// fields
		public String MED_Field_GetName(Pointer p);
		public String MED_Field_GetRefType(Pointer p);
		public int MED_Field_GetType(Pointer p);
		public boolean MED_Field_IsArray(Pointer p);
		public boolean MED_Field_IsAuxPtr(Pointer p);
		public boolean MED_Field_ShowInEditor(Pointer p);
		public String MED_Type_GetInlineEditor(Pointer p);
		
		public int MED_Field_GetArraySize(Pointer field, Pointer mi);
		public void MED_Field_SetArrayIndex(Pointer field, int index);

		public String MED_Field_GetString(Pointer field, Pointer mi);
		public void MED_Field_SetString(Pointer field, Pointer mi, String value);

		public String MED_Field_GetPointer(Pointer field, Pointer mi);
		public void MED_Field_SetPointer(Pointer field, Pointer mi, String value);

		public Pointer MED_Field_GetStructInstance(Pointer field, Pointer mi);
		public long MED_Field_GetInteger(Pointer field, Pointer mi);
		public void MED_Field_SetInteger(Pointer field, Pointer mi, long value);

		public float MED_Field_GetFloat(Pointer field, Pointer mi);
		public void MED_Field_SetFloat(Pointer field, Pointer mi, float value);
		
		public String MED_Field_GetEnumPossibility(Pointer field, int i);
		public String MED_Field_GetEnum(Pointer field, Pointer mi);
		public void MED_Field_SetEnum(Pointer field, Pointer mi, String value);
	
		public void MED_Field_ArrayInsert(Pointer field, Pointer mi);
		public void MED_Field_ArrayErase(Pointer field, Pointer mi);
		
		public Pointer MED_CreateAuxInstance(Pointer onto, Pointer type);
	}

    public static final int FT_INT32 = 0;
    public static final int FT_BYTE = 1;
    public static final int FT_STRING = 2;
    public static final int FT_POINTER = 3;
    public static final int FT_PATH = 4;
    public static final int FT_STRUCT_INSTANCE = 5;
    public static final int FT_FILE = 7;
    public static final int FT_BOOL = 8;
    public static final int FT_FLOAT = 9;
    public static final int FT_ENUM = 10;
    public static final int FT_UINT32 = 11;
    public static final int FT_INVALID = 12;
	
	public static class Field
	{
		Pointer _p;
		
		public Field(Pointer p)
		{
			_p = p;
		}
		
		public String getName()
		{
			return s_ni.MED_Field_GetName(_p);
		}
		
		public void setArrayIndex(int i)
		{
			s_ni.MED_Field_SetArrayIndex(_p,  i);
		}
		
		public boolean isArray()
		{
			return s_ni.MED_Field_IsArray(_p);
		}
		
		public boolean isAuxPtr()
		{
			return s_ni.MED_Field_IsAuxPtr(_p);
		}	
		
		public int getArraySize(MemInstance mi)
		{
			return s_ni.MED_Field_GetArraySize(_p, mi._p);
		}
		
		public String getString(MemInstance mi)
		{
			return s_ni.MED_Field_GetString(_p, mi._p);
		}
		
		public float getFloat(MemInstance mi)
		{
			return s_ni.MED_Field_GetFloat(_p, mi._p);
		}
		
		public void setString(MemInstance mi, String value)
		{
			s_ni.MED_Field_SetString(_p, mi._p, value);
			mi.onModified();
		}
		
		public void setPointer(MemInstance mi, String value)
		{
			s_ni.MED_Field_SetPointer(_p, mi._p, value);
			mi.onModified();
		}
		
		public void setFloat(MemInstance mi, float value)
		{
			s_ni.MED_Field_SetFloat(_p, mi._p, value);
			mi.onModified();
		}
		
		public void setInteger(MemInstance mi, long value)
		{
			s_ni.MED_Field_SetInteger(_p, mi._p, value);
			mi.onModified();
		}
		
		public void setEnum(MemInstance mi, String value)
		{
			s_ni.MED_Field_SetEnum(_p,  mi._p, value);
			mi.onModified();
		}		
		
		public long getInteger(MemInstance mi)
		{
			return s_ni.MED_Field_GetInteger(_p, mi._p);
		}
		
		public String getEnum(MemInstance mi)
		{
			return s_ni.MED_Field_GetEnum(_p,  mi._p);
		}
		
		public String getEnumPossibility(int i)
		{
			return s_ni.MED_Field_GetEnumPossibility(_p,  i);
		}
	
		public String getRefType()
		{
			return s_ni.MED_Field_GetRefType(_p);
		}
		
		public String getPointer(MemInstance mi)
		{
			return s_ni.MED_Field_GetPointer(_p, mi._p);
		}	
		
		public boolean showInEditor()
		{
			return s_ni.MED_Field_ShowInEditor(_p);
		}
		
		public MemInstance getStructInstance(MemInstance mi)
		{
			return new MemInstance(s_ni.MED_Field_GetStructInstance(_p, mi._p));
		}
		
		public void arrayInsert(MemInstance mi)
		{
			s_ni.MED_Field_ArrayInsert(_p,  mi._p);
			mi.onModified();
		}
		
		public void arrayErase(MemInstance mi)
		{
			s_ni.MED_Field_ArrayErase(_p,  mi._p);
			mi.onModified();
		}
		
		public int getType()
		{
			return s_ni.MED_Field_GetType(_p);
		}		
	}
	
	public static class Type 
	{
		Pointer _p;
		
		public Type(Pointer p)
		{
			_p = p;
		}
		
		public String getName()
		{
			return s_ni.MED_Type_GetName(_p); 
		}
		
		public String getModule()
		{
			return s_ni.MED_Type_GetModuleName(_p);
		}
		
		public Field getField(int i)
		{
			return s_wrap.getFieldWrapper(s_ni.MED_Type_GetField(_p,  i));
		}
		
		public Type getParent()
		{
			Pointer p = s_ni.MED_Type_GetParentType(_p);
			if (p == Pointer.NULL)
				return null;
			return s_wrap.getTypeWrapper(p);
		}
		
		public boolean permitAsAuxInstance()
		{
			return s_ni.MED_Type_PermitAsAuxInstance(_p) != 0;
		}
		
		public boolean premitAsAsset()
		{
			return s_ni.MED_Type_PermitAsAsset(_p) != 0;
		}
		
		public String getInlineEditor()
		{
			return s_ni.MED_Type_GetInlineEditor(_p);
		}		
		
		public MemInstance createInstance(String path)
		{
			Pointer p = s_ni.MED_CreateInstance(path, _p);
			MemInstance mi = new MemInstance(p);
			mi.diskSave();
			return s_wrap.load(path);
		}
		
		public boolean hasParent(Type t)
		{
			String name = t.getName();
			Type test = this;
			while (test != null)
			{
				if (test.getName().equals(name))
				{
					return true;
				}
				test = test.getParent();
			}
			return false;
		}
	}
		
	public static class MemInstance
	{
		public Pointer _p;
		public Type _type;
		public boolean _hasUnsavedChanges;
		private int _version;
		
		public MemInstance(Pointer p)
		{
			_p = p;
			_type = s_wrap.getTypeWrapper(s_ni.MED_TypeOf(p));
			_version = 0;
		}
		
		public Type getType()
		{
			return _type;
		}
		
		public Field getField(int i)
		{
			return _type.getField(i);
		}		
		
		public MemInstance createAuxInstance(Type t)
		{
			return new MemInstance(s_ni.MED_CreateAuxInstance(_p,  t._p));
		}		
		
		public String getPath()
		{
			return s_ni.MED_PathOf(_p);
		}
		
		public boolean hasUnsavedChanges()
		{
			return _hasUnsavedChanges;
		}
		
		public void diskSave()
		{
			s_ni.MED_DiskSave(_p);
			_hasUnsavedChanges = false;
		}
		
		public String getContentHash()
		{
			return s_ni.MED_ContentHash(_p);
		}
		
		public String buildJSON()
		{
			return s_ni.MED_MakeJSON(_p);
		}
		
		public void onModified()
		{
			// Mostly for live updates.
			s_ni.MED_OnObjectModified(_p);
			_version++;
		}
		
		public int getVersion()
		{
			return _version;
		}
	}

	public static class NIWrap 
	{
		private NI _i;
		private HashMap<Pointer, Field> s_fields = new HashMap<>();
		private HashMap<Pointer, Type> s_types = new HashMap<>();
		
		public NIWrap(NI i)
		{
			_i = i;
		}
		
		public ArrayList<Type> getAllTypes()
		{
			ArrayList<Type> allTypes = new ArrayList<Type>();
			int idx = 0;
			while (true)
			{
				Pointer t = _i.MED_TypeByIndex(idx);
				if (t == Pointer.NULL)
					break;
				allTypes.add(getTypeWrapper(t));
				idx++;
			}			
			return allTypes;
		}
		
		public Type getTypeByName(String name)
		{
			return getTypeWrapper(_i.MED_TypeByName(name));
		}
		
		public MemInstance load(String path)
		{
			Pointer p = s_ni.MED_DiskLoad(path);
			if (p != Pointer.NULL)
			{
				return new MemInstance(p);
			}
			return null;
		}
		
		public Field getFieldWrapper(Pointer p)
		{
			if (p == Pointer.NULL)
				return null;
			
			Field f = s_fields.get(p);
			if (f == null)
			{
				f = new Field(p);
				s_fields.put(p, f);
			}
			return f;
		}
		
		public Type getTypeWrapper(Pointer p)
		{
			if (p == Pointer.NULL)
				return null;
			
			Type f = s_types.get(p);
			if (f == null)
			{
				f = new Type(p);
				s_types.put(p, f);
			}
			return f;
		}
	}
	
	public static NI s_ni;
	public static NIWrap s_wrap;
	public static String s_resPath;
	public static String s_objsPath;
	
	public static Type getTypeByName(String name)
	{
		return s_wrap.getTypeByName(name);
	}
	
	public static String getObjsPath()
	{
		return s_objsPath;
	}
	
	public static String getResPath()
	{
		return s_resPath;
	}
	
	public static String translateResPath(String path)
	{
		File f = new File(s_resPath, path);
		return f.getAbsolutePath();
	}
	
	public static void writeResFile(File source, String path) throws IOException
	{
		File target = new File(translateResPath(path));
		Files.copy(source.toPath(), target.toPath());
	}
		
	public static boolean Load(String file)
	{
		s_ni = (NI) Native.loadLibrary(file, NI.class);
		s_wrap = new NIWrap(s_ni);
		return s_ni != null;
	}
	
	public static void Initialize(String dllPath, String dataPath)
	{
		s_ni.MED_Initialize(dllPath, dataPath);
		s_resPath = dataPath + "/data/res/";
		s_objsPath = dataPath + "/data/objs/";
	}
}
