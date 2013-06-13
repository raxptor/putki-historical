
	dofile "../libz/premake.lua"

	project "libpng"
		kind "StaticLib"
		language "c"
		targetname "png"
		files { "*.c", "*.h" }
		excludes { "example.c" }
		defines {"PNGLCONF_H"}
		includedirs {"../libz" }
		links {"libz"}


		
