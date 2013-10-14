namespace CCGUI
{
	public interface EventHandler
	{
		void OnEvent(string name);
	}
	
	public class UIRenderContext
	{
		// Applied to final coordinates.
		public float LayoutScale;
		public float LayoutOffsetX;
		public float LayoutOffsetY;
		public UITextureManager TextureManager;
		public UIInputManager InputManager;
		public EventHandler EventHandler; // app defined

		public void LocalToLayout(float x, float y, out float X, out float Y)
		{
			X = LayoutOffsetX + LayoutScale * x;
			Y = LayoutOffsetY + LayoutScale * y;
		}
	}
}
