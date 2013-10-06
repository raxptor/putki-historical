solution "ExampleUI"

	configurations {"Release", "Debug"}
	location "build"
	targetdir "build"
	flags { "Symbols" }
	defines {"_CRT_SECURE_NO_WARNINGS"}

	configuration "Debug"
		defines {"DEBUG"}

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

 if os.get() == "windows" then

	project "example-ui-csharp"

		kind "WindowedApp"
		language "C#"
		targetname "example-ui-csharp"

		files { "src/viewer/**.cs" }
		files { "_gen/outki_csharp/**.cs" }

		links { "ccg-ui-csharp" }
		links { "PresentationFramework", "WindowsBase", "PresentationCore", "System.Xaml", "System" }
	
	end
