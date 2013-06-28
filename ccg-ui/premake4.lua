solution "CCG-UI"

	configurations {"Release", "Debug"}
	location "build"
	targetdir "build"
	flags { "Symbols" }
	defines {"_CRT_SECURE_NO_WARNINGS"}

	configuration "Debug"
		defines {"DEBUG"}

	dofile "../src/putkilib-premake.lua"
	dofile "../external/libpng/premake.lua"
	dofile "../external/freetype-2.5.0.1/premake.lua"

	project "ccg-ui-putki-lib"
		kind "StaticLib"
		language "C++"
		targetname "ccg-ui-putki-lib"
		
		files { "src/types/**.typedef" }
		files { "_gen/inki/**.cpp", "_gen/inki/**.h" }
		files { "_gen/data-dll/**.cpp", "_gen/data-dll/**.h" }

		includedirs { "../src", "../src/builder", "../src/data-dll" }
		includedirs { "_gen" }
		includedirs { "../external/libpng" }

		links {"putki-lib"}

	project "ccg-ui-databuilder"
		kind "ConsoleApp"
		language "C++"
		targetname "ccg-ui-databuilder"

		includedirs { "_gen" }
		includedirs { "../src", "../src/builder" }
		includedirs { "../external/libpng"}
		includedirs { "../external/freetype-2.5.0.1/include"}

		files { "src/putki/builder-main.cpp" }
		files { "src/builder/**.*" }

		files { "../claw/src/builder/binpacker/*.*"}
		includedirs { "../claw/src/"}

		links { "ccg-ui-putki-lib"}
		links { "putki-lib" }
		links { "jsmn" }
		links { "freetype2" }

	project "ccg-ui-data-dll"

		kind "SharedLib"
		language "C++"
		targetname "ccg-ui-data-dll"

		files { "src/putki/dll-main.cpp" }

		includedirs { "../src/builder/**.*" }
		includedirs { "../src/data-dll" }
		includedirs { "../src", "../src/builder/" }
		includedirs { "_gen" }

		links { "putki-lib" }
		links { "ccg-ui-putki-lib"}
		links { "putki-data-dll-lib" }
		links { "jsmn" }

 if os.get() == "windows" then

	project "ccg-ui-viewer"
		kind "WindowedApp"
		language "C#"
		targetname "ccg-ui-viewer"

		files { "src/viewer/**.*"}
		files { "src/uisystem/**.*"}
		files { "_gen/outki_csharp/**.cs"}
		files { "../src/csharp-runtime/**.cs"}

		links { "PresentationFramework", "WindowsBase", "PresentationCore", "System.Xaml", "System" }

		
end
