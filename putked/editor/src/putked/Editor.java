package putked;

import putked.Interop.MemInstance;
import javafx.scene.Node;

public interface Editor 
{
	String getName();	
	int getPriority();
	boolean canEdit(Interop.Type type);
	Node createUI(MemInstance mi);
}
