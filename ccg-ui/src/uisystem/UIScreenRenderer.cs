using System;

namespace CCGUI
{
	public class UIScreenRenderer
	{
		outki.UIScreen m_screen;

		UIElementRenderer m_rootRenderer;
		UITextureManager m_textureManager;

		public UIScreenRenderer(outki.UIScreen screen, UIWidgetHandler handler)
		{
			m_screen = screen;
			m_rootRenderer = handler.CreateRootRenderer(m_screen.Root);

			m_textureManager = new UITextureManager();

			// Load all associated atlases.
			foreach (outki.Atlas a in m_screen.Atlases)
				m_textureManager.AddAtlas(a);
		}


		public void Draw(float x0, float y0, float x1, float y1)
		{
			// calculate the scale
			bool preserveLayoutAspect = false;
			bool useLayoutScaling = true;
			bool useMatrixScaling = false;

			UIElementLayout el = new UIElementLayout();

			// Compute the actual screen pixels (first iteration), to figure out where on 
			// the display this screen should be layouted.
			if (preserveLayoutAspect)
			{
				float sWidth = (x1 - x0) / m_screen.Root.width;
				float sHeight = (y1 - y0) / m_screen.Root.height;

				// use the lowest size.
				float sScale = sWidth < sHeight ? sWidth : sHeight;

				float tgtWidth = (float)Math.Floor(m_screen.Root.width * sScale);
				float tgtHeight = (float)Math.Floor(m_screen.Root.height * sScale);

				el.x0 = (float)Math.Floor((x0 + x1 - tgtWidth) / 2);
				el.y0 = (float)Math.Floor((y0 + y1 - tgtHeight) / 2);
				el.x1 = el.x0 + tgtWidth;
				el.y1 = el.y0 + tgtHeight;
			}
			else
			{
				el.x0 = x0;
				el.y0 = y0;
				el.x1 = x1;
				el.y1 = y1;
			}

			// Compute the (uniform) scale factor to use for layout/matrix scaling.
			UIRenderContext rctx = new UIRenderContext();
			rctx.TextureManager = m_textureManager;

			if (useLayoutScaling || useMatrixScaling)
			{
				float sWidth = (el.x1 - el.x0) / m_screen.Root.width;
				float sHeight = (el.y1 - el.y0) / m_screen.Root.height;
				float sScale = sWidth < sHeight ? sWidth : sHeight;

				if (m_screen.SnapScale)
				{
					for (int i=0;i<m_screen.ScalingForSnapping.Length;i++)
					{
						if (sScale > m_screen.ScalingForSnapping[i])
						{
							sScale = m_screen.ScalingForSnapping[i];
							break;
						}
					}
				}

				/*
				bool snapScale = true;
				if (snapScale)
				{
					if (sScale > 1) sScale = 1;
					if (sScale > ) sScale = 1;
				}
				 */


				rctx.LayoutScale = sScale;
				rctx.LayoutOffsetX = -(float)Math.Floor(el.x0 * (rctx.LayoutScale - 1));
				rctx.LayoutOffsetY = -(float)Math.Floor(el.y0 * (rctx.LayoutScale - 1));
			}
			else
			{
				rctx.LayoutScale = 1;
				rctx.LayoutOffsetX = 0;
				rctx.LayoutOffsetY = 0;
			}

			// the non scaled, layout rect.
			el.nsx0 = el.x0;
			el.nsy0 = el.y0;
			el.nsx1 = el.x0 + (float) Math.Floor((el.x1 - el.x0) / rctx.LayoutScale);
			el.nsy1 = el.y0 + (float) Math.Floor((el.y1 - el.y0) / rctx.LayoutScale);

			m_rootRenderer.OnLayout(rctx, ref el);
			m_rootRenderer.Render(rctx, ref el);
		}
	}

}