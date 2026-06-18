project "Glad"
    location "%{wks.location}/engine/3rdparty/glad"
    kind "StaticLib"
    language "C"
    staticruntime "off"

    targetdir ("%{prj.location}/bin/" .. outputdir .. "/")
    objdir ("%{prj.location}/obj/" .. outputdir .. "/")

    files
    {
        "include/glad/glad.h",
        "include/KHR/khrplatform.h",
        "src/glad.c"
    }

    includedirs
    {
        "include"
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
