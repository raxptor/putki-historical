package putked;
import java.io.*;
import java.nio.file.Files;

public class DefaultBuildLoader implements EditorPluginDescription 
{
	Class<?> m_class;
	String m_interop, m_user;
	String[] m_extras;
	
	public DefaultBuildLoader(Class<?> c, String interop, String user, String[] extras)
	{
		m_class = c;
		m_interop = interop;
		m_user = user;
		m_extras = extras;
	}
	
	@Override
	public String getName() {
		return "EmbeddedBuildLoader";
	}

	@Override
	public String getVersion() {
		return "1.0";
	}

	@Override
	public PluginType getType() {
		return PluginType.PLUGIN_PROJECT_BUILD;
	}
	
	private InputStream open(String path)
	{
		InputStream is = m_class.getResourceAsStream(path);
		if (is != null)
			return is;
		System.out.println("ERR: Could not load embedded " + path);
		return null;
	}

	@Override
	public void start() {
		try {
			File whereTo = Files.createTempDirectory("loader").toFile();
			File s1 = new File(whereTo, m_interop);
			File s2 = new File(whereTo, m_user);
		    InputStream link1 = open("/putked/native/" + m_interop);		    
		    Files.copy(link1, s1.getAbsoluteFile().toPath());
		    InputStream link2 = open("/native/" + m_user);
		    Files.copy(link2, s2.getAbsoluteFile().toPath());		    
		    for (String extra : m_extras) {
				File s3 = new File(whereTo, m_user);
		    	InputStream link3 = open("/native/" + extra);
		    	Files.copy(link3, s3.getAbsoluteFile().toPath());	
		    }
		    Main.interopInit(s1.getAbsolutePath(),  s2.getAbsolutePath());
		}
		catch (Exception e) {
			System.out.println(e.toString());
		}
	}
}
