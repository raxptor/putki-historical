
	dofile "../src/putkilib-premake.lua"
	dofile "../putked/putked-premake.lua"
	dofile "../external/libpng/premake.lua"
	dofile "../external/freetype-2.5.0.1/premake.lua"

	project "ccg-ui-putki-lib"

		if os.get() == "windows" then
			kind "StaticLib"
		else
			kind "SharedLib"
		end

		language "C++"
		targetname "ccg-ui-putki-lib"
		
		files { "src/types/**.typedef" }
		files { "_gen/*putki-master.cpp", "_gen/inki/**.h", "_gen/data-dll/**.h" }

		includedirs { "src", "../src", "../src/builder", "../src/data-dll" }
		includedirs { "_gen" }
		includedirs { "../external/libpng" }

		links {"putki-lib"}

	project "ccg-ui-databuilder"

		if os.get() == "windows" then
			kind "StaticLib"
		else
			kind "SharedLib"
		end

		language "C++"
		targetname "ccg-ui-databuilder"

		includedirs { "_gen" }
		includedirs { "src", "../src", "../src/builder" }
		includedirs { "../external/libpng"}
		includedirs { "../external/freetype-2.5.0.1/include"}

		files { "src/builder/**.*" }
		files { "src/putki/**.*" }
		files { "src/binpacker/**.*" }

		files { "../claw/src/builder/binpacker/*.*"}
		includedirs { "../claw/src/"}

		links { "ccg-ui-putki-lib"}
		links { "putki-lib" }
		links { "freetype2" }

	project "ccg-runtime"

		kind "StaticLib"

                includedirs { "../src/cpp-runtime" }
                includedirs { "src/cpp-runtime" }
		includedirs { "_gen" }

                files { "src/cpp-runtime/**.cpp" }
		files { "src/cpp-runtime/**.h" }
		files { "_gen/outki/**.cpp" }
		files { "_gen/outki/**.h" }

		language "C++"
		targetname "ccg-runtime"

		links {"putki-runtime-lib"}

  if os.get() == "windows" then

	project "ccg-ui-putked-typelib"

		kind "SharedLib"
		language "C#"
		targetname "ccg-ui-putked-typelib"
		files {"_gen/inki_csharp/**.cs"}
		links {"putked-lib"}

	project "ccg-ui-putked-plugin"

		kind "SharedLib"
		language "C#"
		targetname "ccg-ui-putked-plugin"
		files {"putked/ccgui-putked-plugin/**.cs"}
		links {"ccg-ui-putked-typelib"}
		links {"putked-lib"}
		links {"gtk-sharp"}
		links {"atk-sharp"}
		links {"gdk-sharp"}
		links {"glib-sharp"}
		links {"glade-sharp"}
		links {"libstetic"}
  end

 if os.get() == "windows" then

	project "ccg-ui-csharp"
		kind "SharedLib"
		language "C#"
		targetname "ccg-ui-csharp"

		files { "_gen/outki_csharp/**.cs"}
		files { "_gen/outki_csharp/**.cs"}
		files { "src/uisystem/**.*" }
		files { "src/platform/wpf/**.cs" }
		files { "../src/csharp-runtime/**.cs"}

		links { "PresentationFramework", "WindowsBase", "PresentationCore", "System.Xaml", "System" }
	end

