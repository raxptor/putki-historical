solution "Testproj"
	configurations {"Release"}
	location "build"
	targetdir "build"
	flags { "Symbols" }

	dofile "../src/putkilib-premake.lua"

	project "testapp-putki-lib"
		kind "StaticLib"
		language "C++"
		targetname "testapp-putki-lib"
		
		files { "src/putki/bind.cpp" }
		files { "src/types/**.typedef" }

		files { "_gen/inki/**.cpp", "_gen/inki/**.h" }
		excludes { "_gen/inki/bind-dll.cpp" }
		excludes { "_gen/data-dll/**.*" }

		includedirs { "../src", "../src/builder" }
		links {"putki-lib", "jsmn"}

	project "testapp-databuilder"
		kind "ConsoleApp"
		language "C++"
		targetname "testapp-databuilder"

		includedirs { "_gen" }
		includedirs { "../src", "../src/builder" }
		files { "src/builder/**.*" }
		links { "testapp-putki-lib"}
		links { "putki-databuilder-lib"}
		links { "putki-lib" }
		links { "jsmn" }

	project "testapp-data-dll"
		kind "SharedLib"
		language "C++"
		targetname "testapp-data-dll"

		files { "_gen/data-dll/**.cpp", "_gen/data-dll/**.h" }
		files { "_gen/inki/bind-dll.cpp" }
		files { "src/putki/bind-dll.cpp" }

		files { "../src/data-dll/**.cpp", "../src/data-dll/**.h" }

		includedirs { "../src/data-dll" }
		includedirs { "../src", "../src/builder/" }
		includedirs { "_gen" }
		excludes { "../src/cpp-runtime/**" }

		links {"putki-lib"}
		links {"testapp-putki-lib"}
		links {"testapp-databuilder-lib"}

		configuration "Release"
			defines {"DEBUG"}

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

		configuration "Release"
			defines {"DEBUG", "_CRT_SECURE_NO_WARNINGS"}
