
// This file has been generated by the GUI designer. Do not modify.
namespace PutkEd
{
	public partial class IntEditor
	{
		private global::Gtk.Entry m_tbox;

		protected virtual void Build ()
		{
			global::Stetic.Gui.Initialize (this);
			// Widget PutkEd.IntEditor
			global::Stetic.BinContainer.Attach (this);
			this.Name = "PutkEd.IntEditor";
			// Container child PutkEd.IntEditor.Gtk.Container+ContainerChild
			this.m_tbox = new global::Gtk.Entry ();
			this.m_tbox.CanFocus = true;
			this.m_tbox.Name = "m_tbox";
			this.m_tbox.IsEditable = true;
			this.m_tbox.InvisibleChar = '●';
			this.Add (this.m_tbox);
			if ((this.Child != null)) {
				this.Child.ShowAll ();
			}
			this.Hide ();
		}
	}
}
