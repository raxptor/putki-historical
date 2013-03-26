solution "Putki"
	configurations {"Release"}
	location "build"
	targetdir "build"
	flags { "Symbols" }

	project "jsmn"
		kind "StaticLib"
		language "C"
		files { "../external/jsmn/*.c", "../external/jsmn/*.h"}

	project "testapp-putki-lib"
		kind "StaticLib"
		language "C++"
		files { "_gen/**.cpp", "_gen/**.h" }
		files { "src/**.cpp" }
		excludes { "src/main.cpp" }
		includedirs { "../src", "../src/cpp-runtime", "_gen" }

	project "testapp-databuilder"
		kind "ConsoleApp"
		language "C++"
		targetname "databuilder"
		files { "../src/**.cpp", "../src/**.h" }
		files { "../builder/src/main-osx.cpp" }
		includedirs { "../src", "../src/cpp-runtime/", "../external" }
		links {"jsmn", "testapp-putki-lib"}

		configuration "Release"
			defines {"DEBUG"}

	project "testapp-runtime"
		kind "ConsoleApp"
		language "C++"

		targetname "testapp"
		files { "../src/cpp-runtime/**.cpp", "../src/cpp-runtime/**.h" }
		files { "_gen/outki/**.cpp", "_gen/outki/**.h" }
		files { "src/**.cpp" }

		excludes { "src/build-module.cpp" }
		includedirs { "../src/cpp-runtime/", "_gen" }

		configuration "Release"
			defines {"DEBUG"}
