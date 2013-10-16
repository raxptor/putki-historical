using System;
using System.Collections.Generic;
using System.Text;

namespace CCGUI
{
	class UIDialogContainerRenderer : UIWidgetRenderer
	{
		UIWidgetHandler m_handler;
		float m_plateFade = -1.0f;

		// 
		public UIDialogContainerRenderer(outki.UIWidget widget, UIWidgetHandler handler) : base(widget, handler)
		{
			m_handler = handler;
		}

		public override void OnLayout(UIRenderContext rctx, ref UIElementLayout elementLayout)
		{
			base.OnLayout(rctx, ref elementLayout);

			List<UIDialogManager.DialogInstance> dlgs = UIDialogManager.GetDialogs();
			foreach (UIDialogManager.DialogInstance d in dlgs)
			{
				LayoutDialog(rctx, ref elementLayout, d);
			}
		}

		private void LayoutDialog(UIRenderContext rctx, ref UIElementLayout layout, UIDialogManager.DialogInstance dlg)
		{
			if (dlg.Renderer != null)
				dlg.Renderer.OnLayout(rctx, ref layout);
		}

		override public void Render(UIRenderContext rctx, ref UIElementLayout layout)
		{
			List<UIDialogManager.DialogInstance> dlgs = UIDialogManager.GetDialogs();

			float fadeTarget = 0;
			if (dlgs.Count > 0)
				fadeTarget = 1;

			if (m_plateFade == -1)
			{
				m_plateFade = fadeTarget;
			}
			else
			{
				m_plateFade += rctx.FrameDelta * 5 * (fadeTarget - m_plateFade);
				if (fadeTarget == 1 && m_plateFade > 0.98f)
					m_plateFade = 1.0f;
				else if (fadeTarget == 0 && m_plateFade < 0.02f)
					m_plateFade = 0.0f;
			}

			outki.UIColor plate = new outki.UIColor();
			plate.r = 0;
			plate.g = 0;
			plate.b = 0;
			plate.a = (byte)(128 * m_plateFade);

			UIRenderer.DrawSolidRect(layout.x0, layout.y0, layout.x1, layout.y1, plate);

			base.Render(rctx, ref layout);

			foreach (UIDialogManager.DialogInstance d in dlgs)
			{
				// Reinitialize if have no renderer, or handled by wrong widget..
				if (d.Renderer == null || d.DialogContainerTag != (object)this)
				{
					d.DialogContainerTag = (object)this;
					d.Renderer = m_handler.CreateWidgetRenderer(d.Template);
					LayoutDialog(rctx, ref layout, d);
				}

				d.Renderer.Render(rctx, ref layout);
			}
		}
	}
}
