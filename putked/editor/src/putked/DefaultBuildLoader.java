package putked;
import java.io.*;
import java.nio.file.Files;

public class DefaultBuildLoader implements EditorPluginDescription 
{
	Class<?> m_class;
	String m_interop, m_user, m_builder;
	String[] m_extras;
	
	public DefaultBuildLoader(Class<?> c, String interop, String user, String builder, String[] extras)
	{
		m_class = c;
		m_interop = interop;
		m_user = user;
		m_extras = extras;
		m_builder = builder;
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
    		File s3 = new File(whereTo, m_builder);
    		    System.out.println("Copying interop...");
		    InputStream link1 = open("/putked/native/" + m_interop);		    
		    Files.copy(link1, s1.getAbsoluteFile().toPath());
    		    System.out.println("Copying data dll...");
		    InputStream link2 = open("/native/" + m_user);
		    Files.copy(link2, s2.getAbsoluteFile().toPath());		    
    		    System.out.println("Copying builder exe...");
		    InputStream link3 = open("/native/" + m_builder);
		    Files.copy(link3, s3.getAbsoluteFile().toPath());		    
    		    System.out.println("Copying extras...");
		    for (String extra : m_extras) {
			File sE = new File(whereTo, m_user);
		    	InputStream linkE = open("/native/" + extra);
		    	Files.copy(linkE, sE.getAbsoluteFile().toPath());	
		    }
    		    System.out.println("Doing interop init");
		    Main.interopInit(s1.getAbsolutePath(),  s2.getAbsolutePath(), s3.getAbsolutePath());
		}
		catch (Exception e) {
			System.out.println(e.toString());
		}
	}
}
