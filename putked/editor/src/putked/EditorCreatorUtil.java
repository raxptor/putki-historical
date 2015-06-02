package putked;

import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.Label;
import putked.Interop.Field;
import putked.Interop.MemInstance;

public class EditorCreatorUtil
{		
    public static Node makeArrayFieldLabel(Field fi, int index)
    {
        Label lbl = new Label(fi.getName() + "[" + index + "]");
        lbl.setMinWidth(120);
        return lbl;
    }

    public static Node makeFieldLabel(Field fi)
    {
        Label lbl = new Label(fi.getName());
        lbl.getStyleClass().add("field-label");
        lbl.setAlignment(Pos.CENTER_LEFT);
        return lbl;
    }
    
    public static Node makeInlineEditorHeader(String name)
    {
        Label lbl = new Label(name);
        lbl.getStyleClass().add("field-label");
        lbl.setAlignment(Pos.CENTER_LEFT); 
        return lbl;
    }

    public static Node makeLabel(Field fi, int index)
    {
        if (fi.isArray())
            return makeArrayFieldLabel(fi, index);
        else
            return makeFieldLabel(fi);
    }

    public static String makeInlineAuxTitle(MemInstance mi)
    {
        return mi.getType().getName() + " [" + filterAux(mi.getPath()) + "]";
    }

    public static String makeInlineTitle(MemInstance mi)
    {
        return mi.getPath() + " (" + mi.getType().getName() + ")";
    }
    
    public static String filterAux(String input)
    {
    	for (int i=0;i<input.length();i++)
    		if (input.charAt(i) == '#')
    			return input.substring(i);
    	return input;
    }
}
