solution "PutkEdInteropMono"
	platforms { "x32" }
	flags {"StaticRuntime"}
	configurations "Release"
	project "monoedinterop"

		kind "SharedLib"	
		language "C++"
		targetname "monoed-interop"

		files { "interoplib-mono.cpp" }
		files { "../../src/data-dll/**.h" }
		includedirs { "../../src/data-dll/putki" }

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
