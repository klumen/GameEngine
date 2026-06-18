include "dependencies.lua"

workspace "Lumen"
    location "build"
    architecture "x64"
    startproject "Editor"

    configurations{ "Debug", "Release" } --"Ship"}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "ThirdParty"
    include "engine/3rdparty/spdlog"
    include "engine/3rdparty/simpleini"
    include "engine/3rdparty/glfw"
    include "engine/3rdparty/glad"
    include "engine/3rdparty/glm"
    include "engine/3rdparty/stb"
    include "engine/3rdparty/imgui"
    include "engine/3rdparty/ImGuizmo"
    include "engine/3rdparty/entt"
    include "engine/3rdparty/yaml-cpp"
    include "engine/3rdparty/zlib"
group ""

group "Script"
    include "engine/source/script_runtime"
    include "engine/source/script_editor"
group ""

group "Engine"
    include "engine/source/runtime"
    include "engine/source/editor"
group ""
