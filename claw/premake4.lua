solution "Claw"
	configurations {"Release", "Debug"}
	location "build"
	targetdir "build"
	flags { "Symbols" }
	defines {"_CRT_SECURE_NO_WARNINGS"}

	configuration "Debug"
		defines {"DEBUG"}

	dofile "../src/putkilib-premake.lua"

	project "claw-putki-lib"
		kind "StaticLib"
		language "C++"
		targetname "claw-putki-lib"
		
		files { "src/types/**.typedef" }
		files { "_gen/inki/**.cpp", "_gen/inki/**.h" }
		files { "_gen/data-dll/**.cpp", "_gen/data-dll/**.h" }

		includedirs { "../src", "../src/builder", "../src/data-dll" }
		includedirs { "_gen" }

		links {"putki-lib", "jsmn"}

	project "claw-builder-lib"

		kind "StaticLib"
		language "C++"
		targetname "claw-builder-lib"

		includedirs { "_gen" }
		includedirs { "../src", "../src/builder" }

		files { "src/builder/**.*" }

		links { "claw-putki-lib"}
		links { "putki-databuilder-lib"}
		links { "putki-lib" }
		links { "jsmn" }

	project "claw-databuilder"
		kind "ConsoleApp"
		language "C++"
		targetname "claw-databuilder"

		includedirs { "_gen" }
		includedirs { "../src", "../src/builder" }

		files { "src/putki/builder-main.cpp" }

		links { "putki-databuilder-lib"}
		links { "putki-lib" }
		links { "claw-putki-lib"}
		links { "claw-builder-lib" }

	project "claw-data-dll"

		kind "SharedLib"
		language "C++"
		targetname "claw-data-dll"

		files { "src/putki/dll-main.cpp" }

		includedirs { "../src/builder/**.*" }
		includedirs { "../src/data-dll" }
		includedirs { "../src", "../src/builder/" }
		includedirs { "_gen" }

		links { "putki-lib" }
		links { "claw-putki-lib"}
		links { "claw-builder-lib" }
		links { "putki-data-dll-lib" }

	project "claw-runtime"
		kind "ConsoleApp"
		language "C++"

		targetname "claw"
		files { "../src/cpp-runtime/**.cpp", "../src/cpp-runtime/**.h" }
		files { "_gen/outki/**.cpp", "_gen/outki/**.h" }
		files { "src/**.cpp" }

		excludes { "src/builder/**.*" }
		excludes { "src/putki/**.*" }

		includedirs { "../src/cpp-runtime/", "_gen", "src" }
