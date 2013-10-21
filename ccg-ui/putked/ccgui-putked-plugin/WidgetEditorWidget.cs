using System;
using PutkEd;
using System.Collections.Generic;

namespace ccguiputkedplugin
{
	[System.ComponentModel.ToolboxItem (true)]
	public partial class WidgetEditorWidget : Gtk.Bin
	{
		DLLLoader.MemInstance m_miOriginal;
		DLLLoader.MemInstance m_miWidget;
		inki.UIWidget m_iWidget;

		List<DLLLoader.PutkiField> m_fields = null;

		public WidgetEditorWidget(DLLLoader.MemInstance mi) : base()
		{
			// Save both original & miWidget.
			// In putki these will be same pointers, really, but 
			// field set will be different!
			m_miOriginal = mi;
			m_miWidget = DataHelper.CastUp(mi, DLLLoader.GetTypeByName("UIWidget"));
			m_iWidget = new inki.UIWidget(m_miWidget);

			this.Build ();

			m_title.Text = mi.GetPath();

			WidgetViewer wv = new WidgetViewer(mi);
			m_vbox.Add(wv);

			Console.WriteLine("Widget is " + m_iWidget.get_width() + "x" + m_iWidget.get_height());

			for (int i=0;i<m_iWidget.get_layers_count();i++)
			{
				inki.UIElementLayer layer = m_iWidget.get_layers(i);
				Console.WriteLine("Layer[" + i + "] is " + layer);
				if (layer != null)
				{
					Console.WriteLine("  and it has " + layer.get_elements_count() + " elements");
					for (int j=0;j<layer.get_elements_count();j++)
					{
						inki.UIElement el = layer.resolve_elements(j);
						Console.WriteLine("    element[" + j + "] = " + el);
						if (el != null)
						{
							Console.WriteLine("    => type = " + el.m_mi.GetTypeName());
						}
					}

				}
			}

			Console.WriteLine("End of layer listing (" + m_iWidget.get_layers_count() + " layers)");

			wv.SetSizeRequest((int)m_iWidget.get_width(), (int)m_iWidget.get_height());
		}
	}
}

