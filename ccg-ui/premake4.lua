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

	project "ccg-ui-putki-lib"
		kind "StaticLib"
		language "C++"
		targetname "ccg-ui-putki-lib"
		
		files { "src/types/**.typedef" }
		files { "_gen/inki/**.cpp", "_gen/inki/**.h" }
		files { "_gen/data-dll/**.cpp", "_gen/data-dll/**.h" }

		includedirs { "../src", "../src/builder", "../src/data-dll" }
		includedirs { "_gen" }

		links {"putki-lib", "jsmn"}

	project "ccg-ui-databuilder"
		kind "ConsoleApp"
		language "C++"
		targetname "ccg-ui-databuilder"

		includedirs { "_gen" }
		includedirs { "../src", "../src/builder" }

		files { "src/putki/builder-main.cpp" }

		links { "putki-lib" }
		links { "ccg-ui-putki-lib"}
		links { "jsmn" }

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

