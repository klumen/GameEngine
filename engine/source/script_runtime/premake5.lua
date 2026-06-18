project "ScriptRuntime"
    location "%{wks.location}/engine/source/script_runtime"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.8"

    targetdir ("%{prj.location}/bin/" .. outputdir .. "/")
    objdir ("%{prj.location}/obj/" .. outputdir .. "/")

    files
    {
        "**.cs",
    }
    
    filter "configurations:Debug"
        optimize "Off"
        symbols "Default"

    filter "configurations:Release"
        optimize "On"
        symbols "Default"

    -- filter "configurations:Dist"
    --     optimize "Full"
    --     symbols "Off"
