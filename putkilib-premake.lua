
	PUTKI_PATH = path.getdirectory(_SCRIPT)
	ZLIB_PATH = PUTKI_PATH .. "/external/libz"

	ZLIB_INCLUDES = { ZLIB_PATH }
	PUTKI_LIB_INCLUDES = { PUTKI_PATH .. "/src/", PUTKI_PATH .. "/src/data-dll" }
	PUTKI_RT_INCLUDES = { PUTKI_PATH .. "/src/cpp-runtime" }
	PUTKI_LIB_LINKS = { "putki-lib", "jsmn", "libz" }
	
	function putki_use_builder_lib()
		includedirs ( PUTKI_LIB_INCLUDES )
		links (PUTKI_LIB_LINKS)
	end
	
	function putki_use_runtime_lib()
		includedirs (PUTKI_RT_INCLUDES)
		links {"putki-runtime-lib"}
	end
	
	function putki_typedefs_builder(path, use_impls)
		includedirs ("_gen")
		files { path .. "/**.typedef" }
		if use_impls == true then
			files { "_gen/*putki-master.cpp", "_gen/inki/**.h", "_gen/data-dll/**.h" }
		end
	end
	
	function putki_typedefs_runtime(path, use_impls)
		includedirs ("_gen")
		if use_impls == true then
			files { "_gen/outki/**.cpp" }
		end
		files { "_gen/outki/**.h" }
		files { path .. "/**.typedef" }
	end

	configuration {"windows"}
		defines {"USE_WINSOCK"}
		
	if os.get() == "bsd" or os.get() == "linux" then
		table.insert(PUTKI_LIB_LINKS, "pthread")
	end

	dofile "external/libz/premake.lua"
	
	project "jsmn"

		kind "StaticLib"
		targetname "jsmn"
		language "C++"
		files { "external/jsmn/*.cpp", "external/jsmn/*.h"}
	
	project "putki-lib"

		language "C++"
		targetname "putki-lib"

		kind "StaticLib"

		files { "src/**.cpp", "src/**.h" }
		files { "src/**.cpp", "src/**.h" }	
		files { "builder/src/*.cpp" }
		files { "datatool/src/*.cpp" }
		files { "src/md5/*.*" }
		
		excludes { "src/cpp-runtime/**" }
		includedirs { "src", "external"}

		links {"jsmn"}
		links {"libz"}

		configuration {"windows"}
			links {"ws2_32"}
		configuration {"gmake", "linux"}
			links {"pthread"}

	project "putki-runtime-lib"

		language "C++"
		targetname "putki-runtime-lib"
		kind "StaticLib"	

		files { "src/cpp-runtime/**.cpp", "src/cpp-runtime/**.h" }
		includedirs { "src/cpp-runtime" }

if os.get() == "windows" then

	project "putki-runtime-csharp"
		kind "SharedLib"
		language "C#"
		targetname "putki-runtime-csharp"
		files { "src/csharp-runtime/**.cs"}
		links { "System" }

end
	