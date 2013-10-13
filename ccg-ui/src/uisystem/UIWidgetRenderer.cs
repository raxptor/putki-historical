using System;

namespace CCGUI
{
	public class UIWidgetRenderer : UIElementRenderer
	{
		struct ElementList
		{
			public outki.UIElement element;
			public UIElementLayout layout;
			public UIElementRenderer renderer;
		};

		outki.UIWidget m_data;
		UIWidgetHandler m_handler;

		ElementList[] m_rtData;

		public UIWidgetRenderer(outki.UIWidget widget, UIWidgetHandler handler)
		{
			m_data = widget;
			m_handler = handler;
			Setup(m_data, m_handler);
		}

		public void Setup(outki.UIWidget widget, UIWidgetHandler handler)
		{
			int elements = 0;
			int layers = m_data.layers.Length;
			for (int l = 0; l < layers; l++)
			{
				outki.UIElementLayer layer = m_data.layers[l];
				if (layer == null)
					continue;
				elements += layer.elements.Length;
			}

			//
			int count = 0;
			m_rtData = new ElementList[elements];

			for (int l = 0; l < layers; l++)
			{
				outki.UIElementLayer layer = m_data.layers[l];
				if (layer == null)
					continue;

				for (int i = 0; i < layer.elements.Length; i++)
				{
					m_rtData[count].element = layer.elements[i];
					// can happen during live update.
					if (m_rtData[count].element != null)
					{
						m_rtData[count].renderer = handler.CreateRenderer(layer.elements[i]);
					}

					count++;
				}
			}
		}

		public void LayoutElements(UIRenderContext rctx, float nsx0, float nsy0, float nsx1, float nsy1)
		{
			float expX = (nsx1 - nsx0) - m_data.width;
			float expY = (nsy1 - nsy0) - m_data.height;
			if (expX < 0) expX = 0;
			if (expY < 0) expY = 0;

			for (int i = 0; i < m_rtData.Length; i++)
			{
				outki.UIElement el = m_rtData[i].element;
				if (el == null)
					continue;

				UIElementLayout layout;

				float width  = el.layout.width + el.expansion.width * expX;
				float height = el.layout.height + el.expansion.height * expY;

				layout.nsx0 = nsx0 + el.layout.x + el.expansion.x * expX;
				layout.nsy0 = nsy0 + el.layout.y + el.expansion.y * expY;
				layout.nsx1 = layout.nsx0 + width;
				layout.nsy1 = layout.nsy0 + height;

				layout.x0 = (float) Math.Floor(layout.nsx0 * rctx.LayoutScale + rctx.LayoutOffsetX);
				layout.y0 = (float) Math.Floor(layout.nsy0 * rctx.LayoutScale + rctx.LayoutOffsetY);
				layout.x1 = layout.x0 + (float) Math.Floor(0.5f + width * rctx.LayoutScale);
				layout.y1 = layout.y0 + (float) Math.Floor(0.5f + height * rctx.LayoutScale);
				
				m_rtData[i].layout = layout;
				m_rtData[i].renderer.OnLayout(rctx, ref layout);
			}
		}

		virtual public void OnLayout(UIRenderContext rctx, ref UIElementLayout elementLayout)
		{
			LayoutElements(rctx, elementLayout.nsx0, elementLayout.nsy0, elementLayout.nsx1, elementLayout.nsy1);
		}

		virtual public void Render(UIRenderContext rctx, ref UIElementLayout layout)
		{
			DoLiveUpdate(rctx, ref layout);

			for (int i = 0; i < m_rtData.Length; i++)
			{
				if (m_rtData[i].element != null)
				{
					RenderSubElement(rctx, ref layout, m_rtData[i].element, m_rtData[i].renderer, ref m_rtData[i].layout);
				}
			}
		}

		public void DoLiveUpdate(UIRenderContext rctx, ref UIElementLayout layout)
		{
			// Live update
			bool change = Putki.LiveUpdate.Update(ref m_data);
			for (int i = 0; i < m_rtData.Length; i++)
				change |= Putki.LiveUpdate.IsOld(m_rtData[i].element);

			// reload if changed.
			if (change)
			{
				Setup(m_data, m_handler);
				OnLayout(rctx, ref layout);
			}
		}
	
		// This can be overriden for custom drawing.
		virtual public void RenderSubElement(UIRenderContext rctx, ref UIElementLayout myLayout, outki.UIElement element, UIElementRenderer renderer, ref UIElementLayout layout)
		{
			renderer.Render(rctx, ref layout);
		}
	}
}
