solution "Claw"
	configurations {"Release", "Debug"}
	location "build"
	targetdir "build"
	flags { "Symbols" }

	dofile "../src/putkilib-premake.lua"

	project "claw-putki-lib"
		kind "StaticLib"
		language "C++"
		targetname "claw-putki-lib"
		
		files { "src/putki/bind.cpp" }
		files { "src/types/**.typedef" }

		files { "_gen/inki/**.cpp", "_gen/inki/**.h" }
		excludes { "_gen/inki/bind-dll.cpp" }

		includedirs { "../src", "../src/builder" }
		includedirs { "_gen" }

	project "claw-databuilder-lib"
		kind "StaticLib"
		language "C++"
		targetname "claw-databuilder-lib"

		files { "src/builder/**.cpp", "src/builder/**.h" }
		includedirs { "../src", "_gen" }
		excludes { "../src/cpp-runtime/**" }

		links {"putki-databuilder-lib"}
		links {"claw-putki-lib"}

	project "claw-databuilder"
		kind "ConsoleApp"
		language "C++"
		targetname "claw-databuilder"
		files { "src/builder/claw-builder.cpp"}
		links {"claw-databuilder-lib"}
		links {"putki-databuilder-lib"}
		links {"claw-putki-lib"}

	project "claw-data-dll"
		kind "SharedLib"
		language "C++"
		targetname "claw-data-dll"

		files { "_gen/data-dll/**.cpp", "_gen/data-dll/**.h" }
		files { "_gen/inki/bind-dll.cpp" }
		files { "src/putki/bind-dll.cpp" }

		files { "../src/data-dll/**.cpp", "../src/data-dll/**.h" }

		includedirs { "../src/data-dll" }
		includedirs { "../src", "../src/builder/" }
		includedirs { "_gen" }
		excludes { "../src/cpp-runtime/**" }

		links {"putki-lib"}
		links {"claw-putki-lib"}
		links {"claw-databuilder-lib"}

		configuration "Release"
			defines {"DEBUG"}

	project "claw-runtime"
		kind "WindowedApp"
		language "C++"

		targetname "claw"
		files { "../src/cpp-runtime/**.cpp", "../src/cpp-runtime/**.h" }
		files { "_gen/outki/**.cpp", "_gen/outki/**.h" }
		files { "src/**.cpp", "src/**.h" }

		excludes { "src/builder/**.*" }
		excludes { "src/putki/**.*" }

		includedirs { "../src/cpp-runtime/", "_gen" }
		includedirs { "src" }

		configuration "Release"
			defines {"_CRT_SECURE_NO_WARNINGS"}
		configuration "Debug"
			defines {"DEBUG", "_CRT_SECURE_NO_WARNINGS"}

