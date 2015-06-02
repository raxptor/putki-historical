solution "Tools"

    configurations {"Release"}

    flags { "Symbols" }

    location "build"
    targetdir "build"

    defines("_CRT_SECURE_NO_WARNINGS")
    defines("BUILDER_DEFAULT_RUNTIME=x64")
    defines("LIVEUPDATE_ENABLE")
    defines("PUTKI_ENABLE_LOG")
    defines("KOSMOS_ENABLE_LOG")

    configuration {"linux", "gmake"}
        buildoptions {"-fPIC"}
        buildoptions ("-std=c++11")
    configuration {}

    ------------------------------------
    -- Putki must always come first   --
    ------------------------------------

    dofile "../libs.lua"

    project "test-putki-lib"
        language "C++"
        targetname "test-putki-lib"
        kind "StaticLib"
        putki_use_builder_lib()
        putki_typedefs_builder("src/types", true)

    project "test-databuilder"

        kind "ConsoleApp"
        language "C++"
        targetname "test-databuilder"

        files { "src/builder-main.cpp" }
        files { "src/builder/**.*" }
        links { "test-putki-lib" }
        includedirs { "src" }
        
        putki_use_builder_lib()
        putki_typedefs_builder("src/types", false)

    project "test-data-dll"

        kind "SharedLib"
        language "C++"
        targetname "test-data-dll"

        files { "src/dll-main.cpp" }
        files { "src/builder/**.*" }
        links { "test-putki-lib"}
        includedirs { "src" }

        putki_typedefs_builder("src/types", false)
        putki_use_builder_lib()
        
