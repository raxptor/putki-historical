package putked;

import putked.Interop.Field;
import putked.Interop.MemInstance;

public interface FieldEditorCreator 
{
	// Can return null for arrays.
	FieldEditor createEditor(MemInstance mi, Field field, int index, boolean asArray);
}
