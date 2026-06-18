project "Editor"
    location "%{wks.location}/engine/source/editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{prj.location}/bin/" .. outputdir .. "/")
    objdir ("%{prj.location}/obj/" .. outputdir .. "/")

    files
    {
        "**.h",
        "**.cpp"
    }

    includedirs
    {
        "%{IncludeDir.Lumen}",

        "%{IncludeDir.spdlog}",
        "%{IncludeDir.Simpleini}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.GLM}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.Assimp}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGui}/backends",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.EnTT}",
        "%{IncludeDir.YAML}",
        "%{IncludeDir.zlib}",
        "%{IncludeDir.zlib}/contrib",
        "%{IncludeDir.Mono}",
        "%{IncludeDir.VulkanSDK}"
    }

    prebuildcommands
    {
        "{COPYFILE} %{wks.location}/../engine/3rdparty/assimp/bin/* %{cfg.targetdir}",
        "{COPYFILE} %{wks.location}/../engine/3rdparty/mono/bin/* %{cfg.targetdir}"
    }

    buildoptions
    {
        "/utf-8"
    }

    links
    {
        "Runtime"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines "LUMEN_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "LUMEN_RELEASE"
        runtime "Release"
        optimize "on"
        