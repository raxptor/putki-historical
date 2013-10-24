
	configuration {"windows"}
		defines {"USE_WINSOCK"}

	project "jsmn"
		kind "StaticLib"
		targetname "jsmn"
		language "C++"
		files { "../external/jsmn/*.cpp", "../external/jsmn/*.h"}
	
	project "putki-lib"

		if os.get() == "windows" then
			kind "StaticLib"
		else
			kind "SharedLib"
		end
	
		language "C++"
		targetname "putki-lib"

		files { "../src/**.cpp", "../src/**.h" }
		files { "../src/**.cpp", "../src/**.h" }	
		files { "../builder/src/*.cpp" }

		excludes { "../src/cpp-runtime/**" }
		excludes { "../src/generator/**" }

		includedirs { "../src", "../external", "../external/libpng"}

		links {"jsmn"}
		links {"libpng"}
		links {"libz"}

		configuration {"windows"}
			links {"ws2_32"}

		configuration {"gmake"}
			links {"pthread"}


	project "putki-runtime-lib"

		kind "StaticLib"
	
		language "C++"
		targetname "putki-runtime-lib"

		files { "../src/cpp-runtime/**.cpp", "../src/cpp-runtime/**.h" }
		includedirs { "../src/cpp-runtime" }
