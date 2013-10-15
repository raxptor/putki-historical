using System;

namespace ccguiputkedplugin
{
	public class CCGUIPutkEdPlugin
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
			if (pt.Name == "UIWidget")
			{
				return true;
			}
			return false;
		}

		void LaunchEditor(PutkEd.DLLLoader.MemInstance mi)
		{
			WidgetEditorWindow win = new WidgetEditorWindow();
			win.Show();
		}
	}
}

