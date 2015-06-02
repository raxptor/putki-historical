solution "PutkEdInteropJava"
	platforms { "x64" }
	flags {"StaticRuntime"}
	configurations "Release"
	project "interopjava"

		kind "SharedLib"	
		language "C++"
		targetname "putked-java-interop"

		files { "interoplib-java.cpp" }
		files { "../../src/data-dll/**.h" }
		includedirs { "../../src/data-dll/putki" }
