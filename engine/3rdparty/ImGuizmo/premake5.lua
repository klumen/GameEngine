project "ImGuizmo"
    location "%{wks.location}/engine/3rdparty/ImGuizmo"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{prj.location}/bin/" .. outputdir .. "/")
    objdir ("%{prj.location}/obj/" .. outputdir .. "/")

    files
    {
        "ImGuizmo.h",
        "ImGuizmo.cpp"
    }

    includedirs
    {
        "%{IncludeDir.ImGui}"
    }

    defines
    {
        "IMGUI_DEFINE_MATH_OPERATORS"
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
