using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Editor
{
	public partial class ObjectEditor : UserControl, TypeEditor
	{
		public class RowNode
		{
			public Putki.MemInstance mi;
			public Putki.FieldHandler fh;
			public TypeEditor editor;

			public RowDefinition row;
			public List<RowNode> children = new List<RowNode>();
			public int arrayIndex = -1;
		}

        public void OnConnect(ObjectEditor root)
        {

        }

        public class InsertEvt : RoutedEventArgs
        {
            public InsertEvt(RowNode n)
            {
                node = n;
            }
            public RowNode node;
        }
		
		List<RowNode> m_tree = new List<RowNode>();
		public Putki.MemInstance m_obj = null;
		int m_arrayIndex = 0;

		public ObjectEditor()
		{
			InitializeComponent();
		}

		public Control GetRoot()
		{
			return this;
		}

		public void SetObject(Putki.MemInstance mi, Putki.FieldHandler fi, int arrayIndex)
		{
			// Must be struct.
			Height = 20;
			m_grid.Visibility = Visibility.Collapsed; // when in sub-mode, don't add the grid.            
            m_scroll.Visibility = Visibility.Collapsed;
			// m_label.Content = "Struct instance (" + mi.GetType().GetName() + ")";

            if (fi != null)
            {
                // member somewhere.
                fi.SetArrayIndex(arrayIndex);
                m_obj = fi.GetStructInstance(mi);
            }
            else
                m_obj = mi;

			m_arrayIndex = arrayIndex;
		}

		public void SetObject(Putki.MemInstance mi)
		{
			// m_label.Visibility = Visibility.Collapsed; // nly for structs.
		    m_obj = mi;
            OnStructureChanged();
		}

        public void OnStructureChanged()
        {
            m_grid.Children.Clear();
            m_grid.RowDefinitions.Clear();
            m_tree = GetChildRows();
            Layout(m_tree, 0, 0);
        }

		public List<ObjectEditor.RowNode> GetChildRows()
		{
			int f = 0;            
			List<RowNode> l = new List<RowNode>();
			while (true)
			{
				Putki.FieldHandler fh = m_obj.GetType().GetField(f);
				if (fh != null)
				{
                    if (fh.ShowInEditor())
                    {
                        RowNode e = new RowNode();
                        e.mi = m_obj;
                        e.fh = fh;
                        e.row = new RowDefinition();
                        e.row.Height = GridLength.Auto;
                        e.editor = EditorCreator.MakeEditor(fh, fh.IsArray());
                        e.editor.SetObject(m_obj, fh, 0);
                        e.children = e.editor.GetChildRows();
                        l.Add(e);
                    }
					f++;
				}
				else
				{
					break;
				}
			}
			return l;
		}

        public void InsertClick(RowNode node)
        {
            node.fh.SetArrayIndex(0);
            node.fh.ArrayInsert(node.mi);
            OnStructureChanged();
        }

        public void EraseClick(RowNode node)
        {
            node.fh.SetArrayIndex(node.arrayIndex);
            node.fh.ArrayErase(node.mi);
            OnStructureChanged();
        }

		public int Layout(List<RowNode> list, int rowIndex, int indent)
		{
			int idx = 0;
			foreach (RowNode rn in list)
			{
                if (rn.fh != null && rn.fh.GetName() == "parent")
                {
                    rowIndex = Layout(rn.children, rowIndex, indent);
                    continue;
                }

				Label name = new Label();

                if (rn.fh != null)
                {
                    if (rn.fh.IsArray() && !(rn.editor is ArrayEditor))
                        name.Content = rn.fh.GetName() + "[" + (idx++) + "]";
                    else
                        name.Content = rn.fh.GetName();
                }
                else
                {
                    name.Content = rn.mi.GetPath();
                }

				name.HorizontalAlignment = HorizontalAlignment.Left;
				name.VerticalAlignment = VerticalAlignment.Center;
				name.Margin = new Thickness(indent * 15.0f, 0, 0 ,0);

				m_grid.RowDefinitions.Add(rn.row);
				Grid.SetRow(name, rowIndex);
				Grid.SetColumn(name, 0);

				Control ec = rn.editor.GetRoot();
				Grid.SetRow(ec, rowIndex);
				Grid.SetColumn(ec, 1);
				ec.HorizontalAlignment = HorizontalAlignment.Stretch;

				// If is is array entry
				if (rn.fh != null && rn.fh.IsArray())
				{
					Button b = new Button();

					if (rn.arrayIndex == -1)
					{
						b.Content = "Add";
                        b.Click += delegate { InsertClick(rn); };
					}
					else
					{
						b.Content = "Del";
                        b.Click += delegate { EraseClick(rn); };
					}

					Grid.SetRow(b, rowIndex);
					Grid.SetColumn(b, 2);
					m_grid.Children.Add(b);
				}

                rn.editor.OnConnect(this);

				m_grid.Children.Add(name);
				m_grid.Children.Add(ec);

				rowIndex = Layout(rn.children, rowIndex + 1, indent + 1);
			}
			return rowIndex;
		}
	}
}
