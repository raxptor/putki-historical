using System;
using System.IO;
using System.IO.IsolatedStorage;
using System.Collections.Generic;
using Gtk;

namespace PutkEd
{
	public class PutkEdMain
	{
		public static string s_objPath, s_resPath;
		public static Loader s_loader = null;
		public static FileIndex s_fileIndex = null;
		public static DLLLoader s_dataDll = null;
		public static List<EditorPlugin> s_plugins = new List<EditorPlugin>();

		public static void Main (string[] args)
		{
			Application.Init();

			string configPath = "putked.conf";

			do
			{
				try
				{
					if (args.Length > 0 && args[0] == "--reset")
					{
						// Maybe erase file?.
					}
					else
					{
						IsolatedStorageFileStream nf = new IsolatedStorageFileStream("project-config", FileMode.Open);
						StreamReader nr = new StreamReader(nf);
						configPath = nr.ReadLine();
						nf.Close();
					}
				}
				catch (IOException)
				{

				}

				if (File.Exists(configPath))
					break;

				Gtk.FileChooserDialog fcd = new Gtk.FileChooserDialog("PutkEd - Choose project root", null, FileChooserAction.Open, 
				                                                      "Cancel",ResponseType.Cancel, "Open",ResponseType.Accept);


				FileFilter f = new FileFilter();
				f.Name = "PutkEd Configuration";
				f.AddPattern("putked.config");;
				fcd.AddFilter(f);

				if (fcd.Run() == (int)Gtk.ResponseType.Accept)
					configPath = fcd.Filename;
				fcd.Destroy();

			} while (true);


			int lf = configPath.LastIndexOf("/");
			int lb = configPath.LastIndexOf("\\");

			int wh = lb;
			if (lf > lb)
				wh = lf;

			Console.WriteLine("Setting current directory to [" + configPath.Substring(0, wh) + "]");
			Directory.SetCurrentDirectory(configPath.Substring(0, wh));

			Console.WriteLine("Config is at [" + configPath  + "]");

			s_loader = new Loader();

			s_objPath = s_loader.m_configOpts["datapath"] + "/data/objs";
			s_resPath = s_loader.m_configOpts["datapath"] + "/data/res";

			s_dataDll = new DLLLoader();
			s_dataDll.Load(s_loader.m_configOpts["datadll"], s_loader.m_configOpts["datapath"]);

			s_fileIndex = new FileIndex();
			s_fileIndex.Load(s_objPath);

			// -------------------------------------------------------
			// If we made it all the way here, store the config!

			try
			{
				// Store for later.
				IsolatedStorageFileStream p = new IsolatedStorageFileStream("project-config", FileMode.OpenOrCreate);
				StreamWriter wr = new StreamWriter(p);
				wr.WriteLine(configPath);
				wr.Flush();
				p.Close();
			}
			catch (Exception)
			{
				Console.WriteLine("Failing to store configuration path");
			}

		
			Type ti = typeof(EditorPlugin);
			foreach (System.Reflection.Assembly asm in AppDomain.CurrentDomain.GetAssemblies())
			{
				foreach (Type t in asm.GetTypes())
				{
					if (ti.IsAssignableFrom(t) && !t.IsInterface)
					{
						Console.WriteLine("I got the type [" + t.Name + "]");
						s_plugins.Add((EditorPlugin) Activator.CreateInstance(t));
					}
				}
			}


			MainWindow win = new MainWindow();
			win.Show();

			Application.Run ();
		}
	}
}


