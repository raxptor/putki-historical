PUTKI_PATH = path.getdirectory(_SCRIPT)

function putki_use_runtime_lib()
	PUTKI_RT_INCLUDES = { PUTKI_PATH .. "/src/cpp-runtime" }
	includedirs (PUTKI_RT_INCLUDES)
        links {"putki-runtime-lib"}
end

function putki_typedefs_runtime(path, use_impls, pathbase)
	if pathbase == nil then
		pathbase = "."
	end
	
	includedirs (pathbase .. "/_gen")
	if use_impls == true then
		files {pathbase .. "/_gen/*runtime-master.cpp" }
	end

	files { pathbase .. "/_gen/outki/**.h" }
	files { pathbase .. "/_gen/netki/**.h" }
	files { pathbase .. "/" .. path .. "/**.typedef" }
end

function putki_typedefs_runtime_csharp(path, use_impls, pathbase)
	if pathbase == nil then
		pathbase = "."
	end
	
	includedirs (pathbase .. "/_gen")
	if use_impls == true then
		files {pathbase .. "/_gen/*.cs" }
	end

	files { pathbase .. "/" .. path .. "/**.typedef" }
end

project "putki-runtime-lib"
	language "C++"
	targetname "putki-runtime-lib"
	kind "StaticLib"	
	files { "src/cpp-runtime/**.cpp", "src/cpp-runtime/**.h" }
	includedirs { "src/cpp-runtime" }
