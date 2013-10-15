using System;
using System.Collections.Generic;
using System.IO;

namespace PutkEd
{
	public class Loader
	{
		public Dictionary<string, string> m_configOpts = new Dictionary<string, string>();

		public Loader()
		{
			try 
			{
				TextReader tr = new StreamReader("putked.config");

				while (tr != null)
				{
					string line = tr.ReadLine();
					if (line == null)
						break;

					int pos = line.IndexOf('=');
					if (pos >= 0)
					{
						string opt = line.Substring(0, pos);
						string val = line.Substring(pos + 1);
						m_configOpts.Add(opt, val);
					}

					Console.WriteLine("configuration [" + line + "]");
				}
			}
			catch (Exception e)
			{
				Console.WriteLine("Error! " + e.ToString());
	        			throw;
			}
		}
	}
}

