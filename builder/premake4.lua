solution "Putki"
	configurations {"Release", "Debug"}
	location "build"
	targetdir "build"

	project "jsmn"
		kind "StaticLib"
		language "C++"
		files { "../external/jsmn/*.c", "../external/jsmn/*.h"}

	project "Databuilder"
		kind "ConsoleApp"
		language "C++"
		targetname "databuilder"
		files { "../src/**.cpp", "../src/**.h" }
		files { "src/main-osx.cpp" }
		includedirs { "../src", "../src/cpp-runtime/", "../external" }
		links {"json-parser"}

		configuration "Debug"
			defines {"DEBUG"}


