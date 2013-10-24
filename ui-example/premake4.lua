solution "ExampleUI"

	platforms { "x32" }

	configurations {"Release", "Debug"}

	location "build"
	targetdir "build"
	flags { "Symbols" }
	defines {"_CRT_SECURE_NO_WARNINGS"}

	if os.get() == "windows" then
		flags {"StaticRuntime"}
	end

	configuration "Debug"
		defines {"DEBUG"}

	project "prebuild-dummy" 
		language "C"
		kind "SharedLib"
		files { "src/prebuilddummy.c" }

		if os.get() == "windows" then
			prebuildcommands { "..\\compiler\\build\\compiler" }
		else
			prebuildcommands { "cd " .. _WORKING_DIR .. " && ../compiler/build/compiler " }
		end

	dofile "../ccg-ui/ccg-ui-libs.lua"

	project "example-putki-lib"

		if os.get() == "windows" then
			kind "StaticLib"
		else
			kind "SharedLib"
		end

		language "C++"

		targetname "example-putki-lib"
		
		files { "src/types/**.typedef" }
		files {  "_gen/inki/**.h", "_gen/data-dll/**.h" }
		files { "_gen/*-putki-master.cpp",  }

		includedirs { "src", "src/builder", "src/data-dll" }
		includedirs { "_gen" }

		includedirs { "../ccg-ui/src", "../ccg-ui/src/builder", "../ccg-ui-/src/data-dll" }
		includedirs { "../ccg-ui/_gen" }

		includedirs { "../src" }
		includedirs { "../src/data-dll" }
		
		links {"ccg-ui-putki-lib"}
		links {"putki-lib"}

	project "ui-example-databuilder"

		kind "ConsoleApp"
		language "C++"
		targetname "ui-example-databuilder"

		includedirs { "_gen" }
		includedirs { "../ccg-ui/_gen" }
		includedirs { "../src" }

		files { "src/putki/builder-main.cpp" }
		files { "src/builder/**.*" }

		links { "example-putki-lib"}
		links { "ccg-ui-databuilder"}
		links { "ccg-ui-putki-lib"}
		links { "putki-lib"}

	project "ui-example-data-dll"

		kind "SharedLib"
		language "C++"
		targetname "ui-example-data-dll"

		files { "src/putki/dll-main.cpp" }

		includedirs { "../ccg-ui/_gen" }
		includedirs { "_gen" }

		includedirs { "../src" }
		includedirs { "../src/data-dll" }

		links { "example-putki-lib"}
		links { "ccg-ui-databuilder"}
		links { "ccg-ui-putki-lib"}
		links { "putki-lib"}

  if os.get() == "windows" then

	project "ui-example-putked-typelib"

		kind "SharedLib"
		language "C#"
		targetname "ui-example-putked-typelib"
		files {"_gen/inki_csharp/**.cs"}
		links {"ccg-ui-putked-typelib"}
		links {"putked-lib"}
		
	project "example-ui-csharp"

		kind "WindowedApp"
		language "C#"
		targetname "example-ui-csharp"

		files { "src/viewer/**.cs" }
		files { "_gen/outki_csharp/**.cs" }

		links { "ccg-ui-csharp" }
		links { "PresentationFramework", "WindowsBase", "PresentationCore", "System.Xaml", "System" }
	
	end
