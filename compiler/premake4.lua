solution "Compiler"
	configurations {"Release"}
	location "build"
	targetdir "build"

	configuration "Release"
		flags { "NoPCH", "NoRTTI", "Symbols"}

	project "Compiler"
		kind "ConsoleApp"
		language "C++"
		targetname "compiler"
		files { "../src/putki/sys/**.cpp", "../src/parser/**.cpp", "../src/generator/**.cpp", "../src/generator/**.h", "../src/compiler/**.cpp", "../src/compiler/**.h" }
		files { "../src/parser/**.h", "../src/generator/**.h" }
		files { "../src/putki/**.h" }

		files { "src/main-osx.cpp" }
		includedirs { "../src", "../src/cpp-runtime/", "../external" }

		configuration "Release"
			defines {"DEBUG"}


