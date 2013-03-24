solution "Putki"
	configurations {"Release"}
	location "build"
	targetdir "build"

	project "json-parser"
		kind "StaticLib"
		language "C++"
		files { "../external/json-parser/json.c" }

	project "testapp-putki"
		kind "StaticLib"
		language "C++"
		files { "_gen/**.cpp", "_gen/**.h" }
		files { "src/**.cpp" }
		includedirs { "../src", "../src/cpp-runtime", "_gen" }

	project "databuilder"
		kind "ConsoleApp"
		language "C++"
		targetname "databuilder"
		files { "../src/**.cpp", "../src/**.h" }
		files { "../builder/src/main-osx.cpp" }
		includedirs { "../src", "../src/cpp-runtime/", "../external" }
		links {"json-parser", "testapp-putki"}

		configuration "Release"
			defines {"DEBUG"}


