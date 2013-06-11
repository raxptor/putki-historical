	project "jsmn"
		kind "StaticLib"
		targetname "jsmn"
		language "C"
		files { "../external/jsmn/*.c", "../external/jsmn/*.h"}
	
	project "putki-lib"
		kind "StaticLib"
		language "C++"
		targetname "putki-lib"
		files { "../src/**.cpp", "../src/**.h" }
		excludes { "../src/data-dll/**" }
		excludes { "../src/cpp-runtime/**" }
		excludes { "../src/generator/**" }
		includedirs { "../src", "../external" }
		links {"jsmn"}

	project "putki-databuilder-lib"
		kind "StaticLib"
		language "C++"
		targetname "putki-databuilder-lib"
		files { "../builder/src/*.cpp" }
		includedirs { "../src", "../external" }
		links {"jsmn", "putki-lib"}
