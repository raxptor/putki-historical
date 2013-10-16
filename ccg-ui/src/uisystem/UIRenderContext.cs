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
		public float FrameDelta;
		public UITextureManager TextureManager;
		public UIInputManager InputManager;
		public EventHandler EventHandler; // app defined
	}
}
