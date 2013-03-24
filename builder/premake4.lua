solution "Putki"
	configurations {"Release"}
	location "build"
	targetdir "build"

	project "json-parser"
		kind "StaticLib"
		language "C++"
		files { "../external/json-parser/json.c" }

	project "Databuilder"
		kind "ConsoleApp"
		language "C++"
		targetname "databuilder"
		files { "../src/**.cpp", "../src/**.h" }
		files { "src/main-osx.cpp" }
		includedirs { "../src", "../src/cpp-runtime/", "../external" }
		links {"json-parser"}

		configuration "Release"
			defines {"DEBUG"}


