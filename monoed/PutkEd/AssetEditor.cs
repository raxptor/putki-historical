using System;

namespace PutkEd
{
	public partial class AssetEditor : Gtk.Window
	{
		DLLLoader.MemInstance m_mi;

		public AssetEditor(DLLLoader.MemInstance instance) : base(Gtk.WindowType.Toplevel)
		{
			m_mi = instance;
			this.Build();

			this.Title = m_mi.GetTypeName() + " editor";
		}
	}
}
