
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["Lumen"] = "%{wks.location}/../engine/source"
IncludeDir["spdlog"] = "%{wks.location}/../engine/3rdparty/spdlog/include"
IncludeDir["Simpleini"] = "%{wks.location}/../engine/3rdparty/simpleini"
IncludeDir["GLFW"] = "%{wks.location}/../engine/3rdparty/glfw/include"
IncludeDir["Glad"] = "%{wks.location}/../engine/3rdparty/glad/include"
IncludeDir["GLM"] = "%{wks.location}/../engine/3rdparty/glm"
IncludeDir["stb"] = "%{wks.location}/../engine/3rdparty/stb"
IncludeDir["Assimp"] = "%{wks.location}/../engine/3rdparty/assimp/include"
IncludeDir["ImGui"] = "%{wks.location}/../engine/3rdparty/imgui"
IncludeDir["ImGuizmo"] = "%{wks.location}/../engine/3rdparty/ImGuizmo"
IncludeDir["EnTT"] = "%{wks.location}/../engine/3rdparty/entt/single_include/entt"
IncludeDir["YAML"] = "%{wks.location}/../engine/3rdparty/yaml-cpp/include"
IncludeDir["zlib"] = "%{wks.location}/../engine/3rdparty/zlib"
IncludeDir["Mono"] = "%{wks.location}/../engine/3rdparty/mono/include"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["Assimp"] = "%{wks.location}/../engine/3rdparty/assimp/lib"
LibraryDir["Mono"] = "%{wks.location}/../engine/3rdparty/mono/lib"
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Assimp"] = "%{LibraryDir.Assimp}/assimp-vc143-mt.lib"
Library["Mono"] = "%{LibraryDir.Mono}/mono-2.0-sgen.lib"
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"