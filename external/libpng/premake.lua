
	dofile "../libz/premake.lua"

	project "libpng"
		kind "StaticLib"
		language "c"
		targetname "libpng"
		files { "*.c", "*.h" }
		excludes { "example.c" }
		includedirs {"../libz" }
		links {"libz"}
