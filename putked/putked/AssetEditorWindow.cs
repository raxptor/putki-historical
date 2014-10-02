using System;
using Gtk;

namespace PutkEd
{
	public partial class AssetEditorWindow : Gtk.Window
	{
		public AssetEditor m_assetEditor;
		DLLLoader.MemInstance m_mi;

		public AssetEditorWindow() : base(Gtk.WindowType.Toplevel)
		{
			this.Build();
			m_assetEditor = new AssetEditor();
			m_frame.Add(m_assetEditor);
			this.SetPosition(WindowPosition.Center);
		}

		public void SetObject(DLLLoader.MemInstance mi)
		{
			this.Title = "[" + mi.GetPath() + "] " + mi.GetTypeName() + " editor";
			m_path.Text = "[" + mi.GetPath() + "]";
			m_assetEditor.SetObject(mi);
			m_mi = mi;
		}

		protected void OnSaveClicked (object sender, EventArgs e)
		{
			Console.WriteLine("saved");
			m_mi.DiskSave();
		}
	}
}

