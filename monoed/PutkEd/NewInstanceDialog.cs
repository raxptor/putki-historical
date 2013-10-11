using Gtk;
using System;
using System.Collections.Generic;

namespace PutkEd
{
	public partial class NewInstanceDialog : Gtk.Dialog
	{
		public DLLLoader.Types m_selectedType = null;

		public NewInstanceDialog()
		{
			this.Build();
		}

		public void FillWithParentsTo(string RefType)
		{
			List<string> types = new List<string>();
			foreach (DLLLoader.Types d in MainClass.s_dataDll.GetTypes())
			{
				if (RefType == d.Name)
				{
					foreach (DLLLoader.Types e in MainClass.s_dataDll.GetTypes())
					{
						if (DLLLoader.HasParent(e, d))
							types.Add(e.Name);
					}
				}

			}

			types.Sort();

			foreach (string s in types)
				m_types.AppendText(s);
		}

		protected void OnOK (object sender, EventArgs e)
		{
			TreeIter i;
			if (m_types.GetActiveIter(out i))
			{
				string t = m_types.Model.GetValue(i, 0).ToString();
				foreach (DLLLoader.Types d in MainClass.s_dataDll.GetTypes())
				{
					if (d.Name == t)
					{
						m_selectedType = d;
						Respond(0);
					}
				}
			}
		}

		protected void OnCancel (object sender, EventArgs e)
		{
			Respond(1);
		}
	}
}

