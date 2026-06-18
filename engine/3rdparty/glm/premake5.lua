project "GLM"  
    location "%{wks.location}/engine/3rdparty/glm"
    kind "Utility"
    language "C++"  
    cppdialect "C++20"  

    files  
    {
        "glm/**.hpp",
        "glm/**.h",
        "glm/**.inl"
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