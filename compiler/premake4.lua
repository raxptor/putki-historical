solution "Putki"
	configurations {"Release"}
	location "build"
	targetdir "build"

	configuration "Release"
		flags { "NoPCH", "NoRTTI", "Symbols"}

	project "json-parser"
		kind "StaticLib"
		language "C++"
		files { "../external/json-parser/json.c" }

	project "Compiler"
		kind "ConsoleApp"
		language "C++"
		targetname "compiler"
		files { "../src/**.cpp", "../src/**.h" }
		files { "src/main-osx.cpp" }
		includedirs { "../src", "../src/cpp-runtime/", "../external" }
		links {"json-parser"}

		configuration "Release"
			defines {"DEBUG"}


