using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Threading;
using System.IO;

namespace ViewerApp
{
	class TetrisFieldWidgetRenderer : CCGUI.UIWidgetRenderer
	{
		outki.TetrisFieldWidget m_widget;
		CCGUI.UIRenderer.Texture[] m_tex = null;

		public TetrisFieldWidgetRenderer(outki.UIWidget widget, CCGUI.UIWidgetHandler handler) : base(widget, handler)
		{
			m_widget = widget as outki.TetrisFieldWidget;
		}

		public override void OnLayout(CCGUI.UIRenderContext rctx, ref CCGUI.UIElementLayout elementLayout)
		{
			base.OnLayout(rctx, ref elementLayout);

			m_tex = new CCGUI.UIRenderer.Texture[m_widget.Blocks.Length];
			for (int i = 0; i < m_widget.Blocks.Length; i++)
				m_tex[i] = rctx.TextureManager.ResolveTexture(m_widget.Blocks[i], rctx.LayoutScale, 0, 0, 1, 1);
		}

		public override void Render(CCGUI.UIRenderContext rctx, ref CCGUI.UIElementLayout layout)
		{
			base.Render(rctx, ref layout);

			float sX = (layout.x1 - layout.x0 - (m_widget.BlocksX - 1) * m_widget.SpacingX) / m_widget.BlocksX;
			float sY = (layout.y1 - layout.y0 - (m_widget.BlocksY - 1) * m_widget.SpacingY) / m_widget.BlocksY;

			System.Random r = new System.Random();
			for (int y = 0; y < m_widget.BlocksY; y++)
			{
				for (int x = 0; x < m_widget.BlocksX; x++)
				{
					float x0 = layout.x0 + sX * x + m_widget.SpacingX * x;
					float y0 = layout.y0 + sY * y + m_widget.SpacingY * y;
					float x1 = x0 + sX;
					float y1 = y0 + sY;

					if (x == m_widget.BlocksX - 1)
						x1 = layout.x1;
					if (y == m_widget.BlocksY - 1)
						y1 = layout.y1;

					int b = r.Next() % 6;
					if (b < m_tex.Length)
						CCGUI.UIRenderer.DrawTexture(m_tex[b], x0, y0, x1, y1);
				}
			}
		}
	}


	public class ViewerWidgetHandler : CCGUI.UIWidgetHandler
	{
		public ViewerWidgetHandler()
		{

		}

		public CCGUI.UIWidgetRenderer CreateWidgetRenderer(outki.UIWidget widget)
		{
			switch (widget._rtti_type)
			{
				case outki.TetrisFieldWidget.TYPE:
					return new TetrisFieldWidgetRenderer(widget, this);
				default:
					break;
			}

			return CCGUI.UIDefaultWidgetHandler.CreateWidgetRenderer(widget, this);
		}

		public CCGUI.UIElementRenderer CreateRenderer(outki.UIElement element)
		{
			return CCGUI.UIDefaultWidgetHandler.CreateElementRenderer(element, this);
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
			Putki.PackageManager.LoadFromBytes(File.ReadAllBytes("packages\\tetris.pkg"), new Loader());
			Putki.PackageManager.LoadFromBytes(File.ReadAllBytes("packages\\dialogs.pkg"), new Loader());

			Putki.LiveUpdateClient lup = null; 
			
			if (args.Length > 1)
				lup = new Putki.LiveUpdateClient("localhost", new Loader());

			DispatcherTimer dtp = new DispatcherTimer();

			RootWindow w = new RootWindow("CCG-UI C# Runtime");
			
			w.AddScreen(new CCGUI.UIScreenRenderer(Putki.PackageManager.Resolve<outki.UIScreen>("tetris/gamescreen"), new ViewerWidgetHandler()));
			w.AddScreen(new CCGUI.UIScreenRenderer(Putki.PackageManager.Resolve<outki.UIScreen>("dialog/containerscreen"), new ViewerWidgetHandler()));

			CCGUI.UIDialogManager.AddDialog(Putki.PackageManager.Resolve<outki.UIWidget>("dialog/widget"), null);


			dtp.Tick += delegate
			{
				w.DeltaTime += 0.010f;
				if (lup != null && lup.Update() || w.DeltaTime > 0.03f)
				{				
					w.InvalidateVisual();
				}
			};

			dtp.Interval = new TimeSpan(0, 0, 0, 0, 10);
			dtp.Start();

			
			w.ShowDialog();
		}
	}
}

	