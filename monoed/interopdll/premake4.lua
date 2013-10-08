solution "MonoEdInterop"

	configurations "Release"

	project "monoedinterop"

		kind "SharedLib"	
		language "C++"
		targetname "monoed-interop"

		files { "interoplib.cpp" }
		files { "../../src/data-dll/**.h" }
		includedirs { "../../src/data-dll/putki" }
