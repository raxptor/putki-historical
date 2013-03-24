solution "Putki"
	configurations {"Release"}
	location "build"

	project "Compiler"
		kind "ConsoleApp"
		language "C++"
		targetname "compiler"
		targetdir "build"
		files { "../src/**.cpp", "../src/**.h" }
		files { "src/main-osx.cpp" }
		includedirs { "../src", "../src/cpp-runtime/" }

		configuration "Release"
			defines {"DEBUG"}


