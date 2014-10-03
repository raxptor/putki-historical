solution "Compiler"
	configurations {"Release"}
	location "build"
	targetdir "build"

	configuration "Release"
		flags { "NoPCH", "NoRTTI", "Symbols"}

	defines {"_CRT_SECURE_NO_WARNINGS"}

	project "Compiler"
		kind "ConsoleApp"
		language "C++"
		targetname "compiler"

		files { "src/**.cpp"}
		files { "src/**.h" }

		files { "../src/putki/sys/**.cpp" }
		files { "../src/putki/runtime.cpp"}
		files { "../src/putki/**.h" }

		includedirs { "src", "../src", "../external" }

