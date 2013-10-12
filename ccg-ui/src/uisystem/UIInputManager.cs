using System;
using System.Collections.Generic;
using System.Text;

namespace CCGUI
{
	public struct MouseInteraction
	{
		public object ObjectOver;
		public object ObjectPressed;
	};
	
	public class UITouchInteraction
	{
		public UITouchInteraction()
		{
			PressedByTouchId = -1;
			StillInside = false;
		}
		
		public bool StillInside;
		public int PressedByTouchId;
	}

	public class UIInputState
	{
		public float MouseX;
		public float MouseY;
		public bool MouseDown, MouseClicked;
		
		public UITusch.Tch[] Touches;
	};

	public class UIInputManager
	{
		public UIInputState m_state = null;
		public MouseInteraction m_mouseInteraction = new MouseInteraction();

		private int m_depth;

		public UIInputManager()
		{

		}

		public void BeginFrame(UIInputState state)
		{
			m_state = state;
			m_depth = 0;
		}

		public void EndFrame()
		{

		}

		public void EnterLayer()
		{
			m_depth++;
		}

		public void LeaveLayer()
		{
			m_depth--;
		}

		public bool MouseHitTest(float x0, float y0, float x1, float y1)
		{
			return InputHitTest(m_state.MouseX, m_state.MouseY, x0, y0, x1, y1);
		}

		public bool InputHitTest(float x, float y, float x0, float y0, float x1, float y1)
		{
			return x >= x0 && y >= y0 && x < x1 && y < y1;
		}
		
		public void TouchHitTest(float x0, float y0, float x1, float y1, ref UITouchInteraction interaction)
		{
			foreach (UITusch.Tch t in m_state.Touches)
			{
				if (t.position.x >= x0 && t.position.y >= y0 && t.position.x < x1 && t.position.y < y1)
				{
					interaction.PressedByTouchId = t.fingerId;
					interaction.StillInside = true;
					return;
				}
				else if (t.fingerId == interaction.PressedByTouchId)
				{
					interaction.StillInside = false;
					return;
				}
			}
			
			interaction.PressedByTouchId = -1;
			interaction.StillInside = false;
		}
	}
}
