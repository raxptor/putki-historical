
// This file has been generated by the GUI designer. Do not modify.
namespace PutkEd
{
	public partial class BoolEditor
	{
		private global::Gtk.CheckButton m_checkbox;

		protected virtual void Build ()
		{
			global::Stetic.Gui.Initialize (this);
			// Widget PutkEd.BoolEditor
			global::Stetic.BinContainer.Attach (this);
			this.Name = "PutkEd.BoolEditor";
			// Container child PutkEd.BoolEditor.Gtk.Container+ContainerChild
			this.m_checkbox = new global::Gtk.CheckButton ();
			this.m_checkbox.CanFocus = true;
			this.m_checkbox.Name = "m_checkbox";
			this.m_checkbox.Label = "";
			this.m_checkbox.Active = true;
			this.m_checkbox.DrawIndicator = true;
			this.m_checkbox.UseUnderline = true;
			this.Add (this.m_checkbox);
			if ((this.Child != null)) {
				this.Child.ShowAll ();
			}
			this.Hide ();
		}
	}
}