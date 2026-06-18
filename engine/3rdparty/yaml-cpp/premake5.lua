project "YAML"
    location "%{wks.location}/engine/3rdparty/yaml-cpp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

	targetdir ("%{prj.location}/bin/" .. outputdir .. "/")
    objdir ("%{prj.location}/obj/" .. outputdir .. "/")

    files
	{
		"src/**.h",
		"src/**.cpp",
		
		"include/**.h"
	}

	includedirs
	{
		"include"
	}

	defines "YAML_CPP_STATIC_DEFINE"

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
