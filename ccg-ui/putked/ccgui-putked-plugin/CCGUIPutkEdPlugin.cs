using System;
using PutkEd;

namespace ccguiputkedplugin
{
	public class CCGUIPutkEdPlugin : PutkEd.EditorPlugin
	{
		public CCGUIPutkEdPlugin ()
		{

		}

		public string GetDescription()
		{
			return "CCGUI-Plugin";
		}

		public bool CanEditType(PutkEd.DLLLoader.Types pt)
		{
			DLLLoader.Types wt = DLLLoader.GetTypeByName("UIWidget");
			if (wt == null)
				return false;

			return DLLLoader.HasParent(pt, wt);
		}

		public void LaunchEditor(PutkEd.DLLLoader.MemInstance mi)
		{
			WidgetEditWindow win = new WidgetEditWindow(mi);
			win.Show();
		}
	}
}

