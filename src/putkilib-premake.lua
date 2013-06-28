	project "jsmn"
		kind "StaticLib"
		targetname "jsmn"
		language "C++"
		files { "../external/jsmn/*.cpp", "../external/jsmn/*.h"}
	
	project "putki-lib"

		kind "SharedLib"
		language "C++"
		targetname "putki-lib"

		files { "../src/**.cpp", "../src/**.h" }
		files { "../builder/src/*.cpp" }

		excludes { "../src/cpp-runtime/**" }
		excludes { "../src/generator/**" }

		includedirs { "../src", "../external", "../external/libpng"}

		links {"jsmn"}
		links {"libpng"}
		links {"libz"}

		configuration {"windows"}
			defines {"USE_WINSOCK"}
			links {"ws2_32"}

		configuration {"gmake"}
			links {"pthread"}

