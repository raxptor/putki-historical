
	configuration {"windows"}
		defines {"USE_WINSOCK"}
		links {"ws2_32"}

	configuration {"gmake"}
		links {"pthread"}

	project "jsmn"
		kind "StaticLib"
		targetname "jsmn"
		language "C++"
		files { "../external/jsmn/*.cpp", "../external/jsmn/*.h"}
	
	project "putki-lib"
		kind "StaticLib"
		language "C++"
		targetname "putki-lib"
		files { "../src/**.cpp", "../src/**.h" }
		files { "../builder/src/*.cpp" }

		excludes { "../src/data-dll/**" }
		excludes { "../src/cpp-runtime/**" }
		excludes { "../src/generator/**" }
		includedirs { "../src", "../external" }
		links {"jsmn"}

	project "putki-data-dll-lib"
		kind "StaticLib"
		language "C++"
		targetname "putki-data-dll"

		files { "../src/data-dll/**.cpp", "../src/data-dll/**.h" }

		includedirs { "../src/builder/**.*" }
		includedirs { "../src/data-dll" }
		includedirs { "../src", "../src/builder/" }

		links { "jsmn" }
		links { "putki-databuilder-lib"}
		links { "putki-lib" }
