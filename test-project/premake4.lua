solution "Testproj"
	configurations {"Release", "Debug"}
	location "build"
	targetdir "build"
	flags { "Symbols" }

	defines {"_CRT_SECURE_NO_WARNINGS"}

	configuration "Debug"
		defines {"DEBUG"}

	dofile "../src/putkilib-premake.lua"

	project "testapp-putki-lib"
		kind "StaticLib"
		language "C++"
		targetname "testapp-putki-lib"
		
		files { "src/types/**.typedef" }
		files { "_gen/inki/**.cpp", "_gen/inki/**.h" }
		files { "_gen/data-dll/**.cpp", "_gen/data-dll/**.h" }

		includedirs { "../src", "../src/builder", "../src/data-dll" }
		includedirs { "_gen" }

		links {"putki-lib" }
		links {"jsmn"}

	project "testapp-databuilder"
		kind "ConsoleApp"
		language "C++"
		targetname "testapp-databuilder"

		includedirs { "_gen" }
		includedirs { "../src", "../src/builder" }

		files { "src/putki/builder-main.cpp" }

		links { "testapp-putki-lib"}
		links { "putki-lib" }
		links { "jsmn" }

	project "testapp-data-dll"

		kind "SharedLib"
		language "C++"
		targetname "testapp-data-dll"

		files { "src/putki/dll-main.cpp" }

		includedirs { "../src/builder/**.*" }
		includedirs { "../src/data-dll" }
		includedirs { "../src", "../src/builder/" }
		includedirs { "_gen" }

		links { "testapp-putki-lib"}
		links { "putki-lib" }
		links { "jsmn" }

	project "testapp-runtime"
		kind "ConsoleApp"
		language "C++"

		targetname "testapp"
		files { "../src/cpp-runtime/**.cpp", "../src/cpp-runtime/**.h" }
		files { "_gen/outki/**.cpp", "_gen/outki/**.h" }
		files { "src/**.cpp" }
		excludes { "src/builder/**.*" }
		excludes { "src/putki/**.*" }

		includedirs { "../src/cpp-runtime/", "_gen" }
