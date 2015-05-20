package putked;

public interface EditorPluginDescription
{
	enum PluginType
	{
		PLUGIN_EDITOR,
		PLUGIN_PROJECT_DEV_BUILD,
		PLUGIN_PROJECT_BUILD
	}

	String getName();
	String getVersion();
	PluginType getType();
	void start();
}
