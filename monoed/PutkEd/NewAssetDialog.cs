using Gtk;
using System;
using System.Collections.Generic;

namespace PutkEd
{
	public partial class NewAssetDialog : Gtk.Dialog
	{
		public NewAssetDialog()
		{
			this.Build();

			List<string> types = new List<string>();
			foreach (DLLLoader.Types d in MainClass.s_dataDll.GetTypes())
			{
				types.Add(d.Name);
			}

			types.Sort();
			foreach (string s in types)
				m_types.AppendText(s);		
		}

		protected void OnOK (object sender, EventArgs e)
		{
			if (m_path.Text.Length > 0)
			{

				TreeIter i;
				if (m_types.GetActiveIter(out i))
				{
					string t = m_types.Model.GetValue(i, 0).ToString();
					Console.WriteLine("Creating asset " + t + " with path [" + m_path.Text + "]");

					foreach (DLLLoader.Types d in MainClass.s_dataDll.GetTypes())
					{
						if (d.Name == t)
						{
							DLLLoader.MemInstance mi = DLLLoader.MemInstance.Create(m_path.Text, d);
							PutkEd.AssetEditor ae = new PutkEd.AssetEditor();
							ae.SetObject(mi);
							ae.Show();
							mi.DiskSave();
							Respond(0);
						}
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

