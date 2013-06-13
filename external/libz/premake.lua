	project "libz"
		kind "StaticLib"
		language "c"
		targetname "png"
		files { "*.c", "*.h" }
		excludes { "gzread.c", "gzwrite.c" }


		
