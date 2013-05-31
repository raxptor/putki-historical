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
		excludes { "../src/data-dll/**" }
		excludes { "../src/cpp-runtime/**" }
		excludes { "../src/generator/**" }

		includedirs { "../src", "../external" }
		links {"jsmn"}
		configuration "Release"
			defines {"DEBUG"}

	project "putki-databuilder-lib"
		kind "StaticLib"
		language "C++"
		targetname "putki-databuilder-lib"
		files { "../builder/src/*.cpp" }
		includedirs { "../src", "../external" }
		links {"jsmn", "putki-lib"}
		configuration "Release"
			defines {"DEBUG"}

