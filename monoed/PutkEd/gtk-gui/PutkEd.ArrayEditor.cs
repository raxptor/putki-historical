
// This file has been generated by the GUI designer. Do not modify.
namespace PutkEd
{
	public partial class ArrayEditor
	{
		private global::Gtk.Label m_label;

		protected virtual void Build ()
		{
			global::Stetic.Gui.Initialize (this);
			// Widget PutkEd.ArrayEditor
			global::Stetic.BinContainer.Attach (this);
			this.Name = "PutkEd.ArrayEditor";
			// Container child PutkEd.ArrayEditor.Gtk.Container+ContainerChild
			this.m_label = new global::Gtk.Label ();
			this.m_label.Name = "m_label";
			this.m_label.LabelProp = global::Mono.Unix.Catalog.GetString ("label1");
			this.Add (this.m_label);
			if ((this.Child != null)) {
				this.Child.ShowAll ();
			}
			this.Hide ();
		}
	}
}
