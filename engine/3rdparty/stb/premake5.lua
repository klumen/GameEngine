project "stb"  
    location "%{wks.location}/engine/3rdparty/stb"
    kind "Utility"
    language "C++"  
    cppdialect "C++20"  

    files  
    {
        "*.h"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"