project "spdlog"
    location "%{wks.location}/engine/3rdparty/spdlog"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{prj.location}/bin/" .. outputdir .. "/")
    objdir ("%{prj.location}/obj/" .. outputdir .. "/")

    files
    {
        "src/**.cpp",
        "include/spdlog/**.h"
    }

    includedirs
    {
        "include"
    }

    defines "SPDLOG_COMPILED_LIB"

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
