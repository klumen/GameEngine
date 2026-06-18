project "EnTT"
    location "%{wks.location}/engine/3rdparty/entt"
    kind "Utility"
    language "C++"
    cppdialect "C++20"

	files
	{
        "single_include/entt/entt.hpp"
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
