using System;
using System.Collections.Generic;
using System.Text;

namespace CCGUI
{
	static public class UIDialogManager
	{
		public class DialogInstance
		{
			public outki.UIWidget Template;
			public UIWidgetRenderer Renderer;
			public EventHandler EventHandler;
			public object DialogContainerTag;
		};

		private static List<DialogInstance> m_dialogs = new List<DialogInstance>();

		public static void AddDialog(outki.UIWidget widget, EventHandler evt)
		{
			DialogInstance dlg = new DialogInstance();
			dlg.Template = widget;
			dlg.Renderer = null;
			dlg.EventHandler = evt;
			m_dialogs.Add(dlg);
		}

		public static List<DialogInstance> GetDialogs()
		{
			return m_dialogs;
		}
	}
}
