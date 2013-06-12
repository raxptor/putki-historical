solution "PutkEd"
	configurations {"Debug", "Release"}

	location "build"
	targetdir "build"

	include "wpf/Editor/premake4.lua"

	project "EditorBridge"
		kind "SharedLib"
		language "C++"
		flags "Managed"
		targetname "editorbridge"

		files { "src/**.cpp", "src/**.h" }
		includedirs { "../src", "../src/cpp-runtime/", "../src/data-dll", "../external" }

		configuration "Debug"
			defines {"Debug"}
