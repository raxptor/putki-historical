namespace CCGUI
{

	public struct UIElementLayout
	{
		// these are the real coordinates for a layout that has been scaled.
		public float x0, y0, x1, y1;

		// these are the layout coordinates 'pretend, unscaled space'
		public float nsx0, nsy0, nsx1, nsy1;
	};

	interface UIElementRenderer
	{
		// Guaranteed to come at least once, and before any rendering takes place.
		// This is a good place to load in textures for the appropriate resolution.
		void OnLayout(UIRenderContext rctx, ref UIElementLayout elementLayout);

		// Do the actual drawing.
		void Render(UIRenderContext rctx, ref UIElementLayout elementLayout);
	}
}