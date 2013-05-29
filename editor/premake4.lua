solution "Putki"
	configurations {"Debug"}
	location "build"
	targetdir "build"

	project "EditorBridge"
		kind "SharedLib"
		language "C++"
		flags "Managed"
		targetname "editorbridge"
		includedirs { "../src", "../src/cpp-runtime/", "../external" }

		configuration "Debug"
			defines {"Debug"}


