PUTKI_PATH = path.getdirectory(_SCRIPT)
ZLIB_PATH = PUTKI_PATH .. "/external/libz"
ZLIB_INCLUDES = { ZLIB_PATH }
PUTKI_LIB_INCLUDES = { PUTKI_PATH .. "/src/", PUTKI_PATH .. "/src/data-dll" }
PUTKI_LIB_LINKS = { "putki-lib", "jsmn", "libz" }

function putki_use_builder_lib()
	includedirs ( PUTKI_LIB_INCLUDES )
	links (PUTKI_LIB_LINKS)
	configuration {"windows"}
		links {"ws2_32"}
	configuration {"gmake", "linux"}
		links {"pthread"}
	configuration {}
end

function putki_typedefs_builder(path, use_impls, pathbase)
	if pathbase == nil then
		pathbase = "."
	end
	includedirs (pathbase .. "/_gen")
	files { pathbase .. "/" .. path .. "/**.typedef" }
	if use_impls == true then
		files { pathbase .. "/_gen/*inki-master.cpp", pathbase .. "/_gen/inki/**.h", pathbase ..  "/_gen/editor/**.h" }
	end
end

defines { "JSMN_PARENT_LINKS" }

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


if os.get() == "windows" and false then
	project "putki-runtime-csharp"
		kind "SharedLib"
		language "C#"
		targetname "putki-runtime-csharp"
		files { "src/csharp-runtime/**.cs"}
		links { "System" }

end
	