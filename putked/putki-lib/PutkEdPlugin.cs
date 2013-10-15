namespace PutkEd
{
	public interface EditorPlugin
	{
		string GetDescription();
		bool CanEditType(DLLLoader.Types type);
		void LaunchEditor(DLLLoader.MemInstance mi);
	}
}

