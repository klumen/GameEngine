project "ScriptEditor"
    location "%{wks.location}/engine/source/script_editor"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.8"

    targetdir ("%{wks.location}/engine/source/editor/resource/scripts")
    objdir ("%{wks.location}/engine/source/editor/resource/scripts/Intermediates")

    files
    {
        "**.cs",
    }

    links
    {
        "ScriptRuntime"
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
