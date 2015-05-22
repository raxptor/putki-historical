package putked;

import java.util.ArrayList;

public class DataHelper 
{
	public static ArrayList<EditorTypeService> _installed = new ArrayList<>();
	
	public static void addTypeService(EditorTypeService serv)
	{
		_installed.add(serv);
	}
	
	public static ProxyObject createPutkEdObj(Interop.MemInstance mi)
	{
		for (EditorTypeService srv : _installed)
		{
			ProxyObject tmp = srv.createProxy(mi.getType().getName());
			if (tmp != null)
			{
				tmp.connect(mi);
				return tmp;
			}
		}
		return null;
	}
}
