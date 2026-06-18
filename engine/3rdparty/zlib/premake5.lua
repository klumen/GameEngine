project "zlib"
    location "%{wks.location}/engine/3rdparty/zlib"
    kind "StaticLib"
    language "C"
    staticruntime "off"

	targetdir ("%{prj.location}/bin/" .. outputdir .. "/")
    objdir ("%{prj.location}/obj/" .. outputdir .. "/")

    files
	{
        "*.h",
        "*.c",

        "contrib/minizip/*.h",
        "contrib/minizip/*.c"
	}

    includedirs
    {
        "."
    }

    defines
    {
        "_CRT_SECURE_NO_DEPRECATE",
        "_CRT_NONSTDC_NO_DEPRECATE"
    }
    
    buildoptions
    {
        "/utf-8"
    }

	filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
