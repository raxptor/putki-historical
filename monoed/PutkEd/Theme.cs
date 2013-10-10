using System;

namespace PutkEd
{
	public partial class Theme : Gtk.ActionGroup
	{
		public Theme() : 
				base("PutkEd.Theme")
		{
			this.Build();
		}
	}
}

