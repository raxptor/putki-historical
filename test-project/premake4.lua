solution "Putki"
	configurations {"Release"}
	location "build"
	targetdir "build"
	flags { "Symbols" }

	project "jsmn"
		kind "StaticLib"
		language "C"
		files { "../external/jsmn/*.c", "../external/jsmn/*.h"}

	project "putki-databuilder-lib"
		kind "StaticLib"
		language "C++"
		targetname "putki-databuilder-lib"
		files { "../src/**.cpp", "../src/**.h" }
		files { "../builder/src/*.cpp" }
		includedirs { "../src", "../src/cpp-runtime/", "../external" }
		links {"jsmn"}
		configuration "Release"
			defines {"DEBUG"}

	project "testapp-databuilder"
		kind "ConsoleApp"
		language "C++"
		targetname "testapp-databuildel"
		files { "src/**.typedef" }
		files { "_gen/putki/**.cpp", "_gen/putki/**.h" }
		files { "src/builder/**.cpp", "src/builder/**.h" }
		excludes { "src/main.cpp" }
		includedirs { "../src", "../src/cpp-runtime", "_gen" }
		links {"putki-databuilder-lib"}

	project "testapp-runtime"
		kind "ConsoleApp"
		language "C++"

		targetname "testapp"
		files { "../src/cpp-runtime/**.cpp", "../src/cpp-runtime/**.h" }
		files { "_gen/outki/**.cpp", "_gen/outki/**.h" }
		files { "src/**.cpp" }
		excludes { "src/builder/**.*" }

		includedirs { "../src/cpp-runtime/", "_gen" }

		configuration "Release"
			defines {"DEBUG"}
