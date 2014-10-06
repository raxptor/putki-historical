using System;
using Gtk;
using System.Collections.Generic;

namespace PutkEd
{
	[System.ComponentModel.ToolboxItem(true)]
	public partial class TextEditor : Gtk.Window {

		public TextEditor() : base (Gtk.WindowType.Toplevel)
		{
			SetPosition(WindowPosition.CenterAlways);
			this.SetDefaultSize(300, 500);
			SetPosition(WindowPosition.Center);
		}

		public void SetObject(DLLLoader.MemInstance mi, DLLLoader.PutkiField fi, int arrayIndex)
		{
			this.Title = mi.GetPath() + ":" + fi.GetName();
			m_text.Buffer.Text = fi.GetString(mi);
			m_text.Buffer.Changed += delegate {
				fi.SetArrayIndex(0);
				fi.SetString(mi, m_text.Buffer.Text);
			};
		}
	}
}

