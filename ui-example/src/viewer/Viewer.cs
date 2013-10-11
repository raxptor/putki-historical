using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Threading;
using System.IO;

namespace ViewerApp
{
    class SpecialElementRenderer : CCGUI.UIElementRenderer
    {
        outki.SpecialElement m_element;

        public SpecialElementRenderer(outki.SpecialElement element)
        {
            m_element = element;
        }

        public void OnLayout(CCGUI.UIRenderContext rctx, ref CCGUI.UIElementLayout elementLayout)
        {
        }

        public void Render(CCGUI.UIRenderContext rctx, ref CCGUI.UIElementLayout layout)
        {

        }
    }


	public class ViewerWidgetHandler : CCGUI.UIWidgetHandler
	{
		public ViewerWidgetHandler()
		{

		}

		public CCGUI.UIElementRenderer CreateRootRenderer(outki.UIWidget widget)
		{
			return CCGUI.UIDefaultWidgetHandler.CreateRootRenderer(widget, this);
		}

		public CCGUI.UIElementRenderer CreateRenderer(outki.UIElement element)
		{
            switch (element._rtti_type)
            {
                case outki.SpecialElement.TYPE:
                    return new SpecialElementRenderer(element as outki.SpecialElement);
                default:
                    break;
            }
			return CCGUI.UIDefaultWidgetHandler.CreateRenderer(element, this);
		}
	}

    // Package loader, redirects to the two components.
    class Loader : Putki.TypeLoader
    {
        public void ResolveFromPackage(int type, object obj, Putki.Package pkg)
        {
            outki.CCGUIDataLoader.ResolveFromPackage(type, obj, pkg);
            outki.UIExampleDataLoader.ResolveFromPackage(type, obj, pkg);
        }

        public object LoadFromPackage(int type, Putki.PackageReader reader)
        {
            object tmp = outki.CCGUIDataLoader.LoadFromPackage(type, reader);
            if (tmp != null)
                return tmp;

            return outki.UIExampleDataLoader.LoadFromPackage(type, reader);
        }
    }

    class Viewer
    {
		[STAThread]
        static void Main(string[] args)
        {			
			Putki.Package p = Putki.PackageLoader.FromBytes(File.ReadAllBytes("packages\\static.pkg"), new Loader());
			Putki.LiveUpdate.InsertPackage(p);

			outki.UIScreen screen = (outki.UIScreen)p.Resolve("screens/ingame");

			Putki.LiveUpdateClient lup = new Putki.LiveUpdateClient("localhost", new Loader());

			DispatcherTimer dtp = new DispatcherTimer();

			RootWindow w = new RootWindow(new CCGUI.UIScreenRenderer(screen, new ViewerWidgetHandler()));
			

			dtp.Tick += delegate
			{
				if (lup.Update())
					w.InvalidateVisual();
			};

			dtp.Interval = new TimeSpan(0, 0, 0, 0, 10);
			dtp.Start();

			
			w.ShowDialog();


			/*
			outki.ProjectDescription dr = (outki.ProjectDescription)p.Resolve("description");
			Console.WriteLine("Welcome to " + dr.Name);

			outki.UIWidget w = (outki.UIWidget)p.Resolve("widget");
			Console.WriteLine("Widget id is " + w.width);

			outki.Font f = (outki.Font)p.Resolve("fonts/calibri");
			Console.WriteLine("Font is " + f);
				
			outki.Atlas a = (outki.Atlas)p.Resolve("atlas1");
			Console.WriteLine("Atlas 1 is " + a);
			 */
		}
	}
}

	