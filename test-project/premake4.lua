solution "Putki"
	configurations {"Release"}
	location "build"
	targetdir "build"
	flags { "Symbols" }

	project "jsmn"
		kind "StaticLib"
		language "C"
		files { "../external/jsmn/*.c", "../external/jsmn/*.h"}

	project "putki-lib"
		kind "StaticLib"
		language "C++"
		targetname "putki-lib"
		files { "../src/**.cpp", "../src/**.h" }
		files { "../builder/src/*.cpp" }
		excludes { "../src/data-dll/**.*" }

		includedirs { "../src", "../src/cpp-runtime/", "../external" }
		links {"jsmn"}
		configuration "Release"
			defines {"DEBUG"}	

	project "putki-databuilder-lib"
		kind "StaticLib"
		language "C++"
		targetname "putki-databuilder-lib"
		files { "../builder/src/*.cpp" }
		includedirs { "../src", "../src/cpp-runtime/", "../external" }
		links {"jsmn", "putki-lib"}
		configuration "Release"
			defines {"DEBUG"}

	project "testapp-putki-lib"
		kind "StaticLib"
		language "C++"
		targetname "testapp-putki-lib"
		
		files { "src/putki/bind.cpp" }

		files { "src/types/**.typedef" }
		files { "_gen/putki/**.cpp", "_gen/putki/**.h" }
		excludes { "_gen/putki/bind-dll.cpp" }

		includedirs { "../src", "../src/cpp-runtime", "../src/builder" }

	project "testapp-databuilder"
		kind "ConsoleApp"
		language "C++"
		targetname "testapp-databuildel"

		files { "src/builder/**.cpp", "src/builder/**.h" }
		includedirs { "../src", "../src/cpp-runtime", "_gen" }
		links {"putki-databuilder-lib"}
		links {"testapp-putki-lib"}

	project "testapp-data-dll"
		kind "SharedLib"
		language "C++"
		targetname "testapp-data-dll"

		files { "_gen/data-dll/**.cpp", "_gen/data-dll/**.h" }
		files { "_gen/putki/bind-dll.cpp" }
		files { "src/putki/bind-dll.cpp" }

		files { "../src/data-dll/**.cpp", "../src/data-dll/**.h" }

		includedirs { "../src/data-dll" }
		includedirs { "../src", "../src/builder/" }


		links {"putki-lib"}
		links {"testapp-putki-lib"}

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
			defines {"DEBUG"}
