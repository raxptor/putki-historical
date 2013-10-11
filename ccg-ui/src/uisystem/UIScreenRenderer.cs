using System;

namespace CCGUI
{
	public class UIScreenRenderer
	{
		outki.UIScreen m_screen;

		UIElementRenderer m_rootRenderer;
		UITextureManager m_textureManager;
		UIWidgetHandler m_widgetHandler;

		public UIScreenRenderer(outki.UIScreen screen, UIWidgetHandler handler)
		{
			m_screen = screen;
			m_widgetHandler = handler;

			CreateAndInit();
		}

		public void CreateAndInit()
		{
			m_rootRenderer = m_widgetHandler.CreateRootRenderer(m_screen.Root);
			m_textureManager = new UITextureManager();

			// Load all associated atlases.
			foreach (outki.Atlas a in m_screen.Atlases)
			{
				if (a != null)
					m_textureManager.AddAtlas(a);
			}
		}

		public void Draw(float x0, float y0, float x1, float y1, UIInputManager inputManager)
		{
			bool mod = false;
			mod |= Putki.LiveUpdate.Update(ref m_screen);
			mod |= Putki.LiveUpdate.Update(ref m_screen.Root);
			if (mod)
			{
				CreateAndInit();
			}

			// calculate the scale
			bool preserveLayoutAspect = m_screen.Config.PreserveLayoutAspect;
			bool useLayoutScaling = (m_screen.Config.ScaleMode == outki.UIScaleMode.ScaleMode_Prop_Layout);
			bool useMatrixScaling = (m_screen.Config.ScaleMode == outki.UIScaleMode.ScaleMode_Prop_Transform);
			
			UIElementLayout el = new UIElementLayout();

			// Compute the actual screen pixels (first iteration), to figure out where on 
			// the display this screen should be layouted.
			if (preserveLayoutAspect)
			{
				float cutW = m_screen.Root.width - m_screen.Config.CutL - m_screen.Config.CutR;
				float cutH = m_screen.Root.height - m_screen.Config.CutT - m_screen.Config.CutB;
					
				// when layouting here, 
				float sWidth = (x1 - x0) / cutW;
				float sHeight = (y1 - y0) / cutH;

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
			rctx.InputManager = inputManager;
			rctx.TextureManager = m_textureManager;

			if (useLayoutScaling || useMatrixScaling)
			{
				float sWidth = (el.x1 - el.x0) / m_screen.Root.width;
				float sHeight = (el.y1 - el.y0) / m_screen.Root.height;
				float sScale = sWidth < sHeight ? sWidth : sHeight;

				if (m_screen.Config.SnapScale)
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