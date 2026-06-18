project "ImGui"
    location "%{wks.location}/engine/3rdparty/imgui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

	targetdir ("%{prj.location}/bin/" .. outputdir .. "/")
    objdir ("%{prj.location}/obj/" .. outputdir .. "/")

	files
	{
        "*.h",
        "*.cpp",

        "backends/imgui_impl_glfw.cpp",
        "backends/imgui_impl_glfw.h",
        "backends/imgui_impl_opengl3.cpp",
        "backends/imgui_impl_opengl3.h",
        "backends/imgui_impl_opengl3_loader.h",

        "backends/imgui_impl_vulkan.cpp",
        "backends/imgui_impl_vulkan.h"
	}

    includedirs
    {
        ".",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.VulkanSDK}"
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
