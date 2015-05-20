package putked;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

public class ConfigParser
{
	private HashMap<String, ArrayList<String>> m_data = new HashMap<>();
	
	public ConfigParser(File fin) throws IOException
	{
		// Construct BufferedReader from FileReader
		BufferedReader br = new BufferedReader(new FileReader(fin));
		String line = null;
		while ((line = br.readLine()) != null) {
			int p = line.indexOf('=');
			if (p > 0) {
				filterKeyVal(line.substring(0, p), line.substring(p +1 ));
			}
		}
		br.close();
	}
	
	public ArrayList<String> getMulti(String key)
	{
		ArrayList<String> v = m_data.get(key);
		if (v == null)
			return new ArrayList<String>();
		else
			return v;
	}
	
	public String getSingle(String key)
	{
		ArrayList<String> vals = getMulti(key);
		if (vals == null)
			return null;
		return vals.get(vals.size()-1);
	}
	
	private void filterKeyVal(String key, String val)
	{
		int f0 = key.indexOf(':');
		if (f0 >= 0) {
			String platform = key.substring(0, f0);
			if (platform.equals("darwin"))
				addKeyVal(key.substring(f0+1), val);
		} else {
			addKeyVal(key, val);
		}
	}
	
	private void addKeyVal(String key, String val)
	{
		System.out.println("Config[" + key + "] = [" + val + "]");
		if (!m_data.containsKey(key))
			m_data.put(key, new ArrayList<String>());
		m_data.get(key).add(val);
	}
}
