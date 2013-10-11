using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CCGUI
{
	public class UIElementLogic
	{
		public enum ButtonVisualStates
		{
			NORMAL = 0,
			MOUSEOVER = 1,
			PRESSED = 2
		}

		public static void MouseButton(UIInputManager im, object obj, float x0, float y0, float x1, float y1)
		{
			if (!im.m_state.MouseDown)
			{
				if (im.MouseHitTest(x0, y0, x1, y1))
					im.m_mouseInteraction.ObjectOver = obj;
				else if (im.m_mouseInteraction.ObjectOver == obj)
					im.m_mouseInteraction.ObjectOver = obj;
			}

			if (im.m_mouseInteraction.ObjectOver == obj)
			{
				if (im.m_state.MouseDown)
					im.m_mouseInteraction.ObjectPressed = obj;
			}

			if (im.m_mouseInteraction.ObjectPressed == obj)
			{
				if (!im.m_state.MouseDown)
					im.m_mouseInteraction.ObjectPressed = null;
			}
		}

		public static ButtonVisualStates GetButtonVisualState(UIInputManager im, object obj)
		{
			if (im.m_mouseInteraction.ObjectPressed == obj)
				return ButtonVisualStates.PRESSED;
			else if (im.m_mouseInteraction.ObjectOver == obj)
				return ButtonVisualStates.MOUSEOVER;
			else
				return ButtonVisualStates.NORMAL;
		}
	}
}
