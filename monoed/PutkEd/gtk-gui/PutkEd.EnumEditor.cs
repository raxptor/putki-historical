
// This file has been generated by the GUI designer. Do not modify.
namespace PutkEd
{
	public partial class EnumEditor
	{
		private global::Gtk.ComboBox m_combo;

		protected virtual void Build ()
		{
			global::Stetic.Gui.Initialize (this);
			// Widget PutkEd.EnumEditor
			global::Stetic.BinContainer.Attach (this);
			this.Name = "PutkEd.EnumEditor";
			// Container child PutkEd.EnumEditor.Gtk.Container+ContainerChild
			this.m_combo = global::Gtk.ComboBox.NewText ();
			this.m_combo.Name = "m_combo";
			this.Add (this.m_combo);
			if ((this.Child != null)) {
				this.Child.ShowAll ();
			}
			this.Hide ();
		}
	}
}
