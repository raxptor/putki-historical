	project "libz"
		kind "StaticLib"
		language "c"
		targetname "libz"
		files { "*.c", "*.h" }
		excludes { "gzread.c", "gzwrite.c" }


		
