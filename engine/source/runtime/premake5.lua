project "Runtime"
    location "%{wks.location}/engine/source/runtime"
    kind "StaticLib"
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

    buildoptions
    {
        "/utf-8"
    }

    links
    {
        "opengl32.lib",
        "spdlog",
        "Simpleini",
        "GLFW",
        "Glad",
        "%{Library.Assimp}",
        "ImGui",
        "ImGuizmo",
        "EnTT",
        "YAML",
        "zlib",
        "%{Library.Mono}",
        "%{Library.Vulkan}"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines "LUMEN_DEBUG"
        runtime "Debug"
        symbols "on"

        links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

    filter "configurations:Release"
        defines "LUMEN_RELEASE"
        runtime "Release"
        optimize "on"

        links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

    -- filter "configurations:Ship"
