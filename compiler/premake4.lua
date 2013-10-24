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
		files { "../src/putki/sys/**.cpp", "../src/parser/**.cpp", "../src/generator/**.cpp", "../src/generator/**.h", "../src/compiler/**.cpp", "../src/compiler/**.h" }
		files { "../src/putki/runtime.cpp"}
		files { "../src/parser/**.h", "../src/generator/**.h" }
		files { "../src/putki/**.h" }

		files { "src/main.cpp" }
		includedirs { "../src", "../src/cpp-runtime/", "../external" }

