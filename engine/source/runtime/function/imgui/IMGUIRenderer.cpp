#include "runtime/function/imgui/IMGUIRenderer.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/function/render/Window.h"
#include "runtime/resource/config/ConfigManager.h"
#include "runtime/resource/asset/AssetManager.h"
#include "runtime/core/Macro.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

namespace Lumen
{
	void IMGUIRenderer::StartUp()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		float fontSize = 36.f;
		io.Fonts->AddFontFromFileTTF(LUMEN_CONFIG_MANAGER->GetBoldFontFile().string().c_str(),
			fontSize, nullptr, io.Fonts->GetGlyphRangesChineseFull());
		io.FontDefault = io.Fonts->AddFontFromFileTTF(LUMEN_CONFIG_MANAGER->GetRegularFontFile().string().c_str(),
			fontSize, nullptr, io.Fonts->GetGlyphRangesChineseFull());

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer bindings
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::OpenGL:
		{
			ImGui_ImplGlfw_InitForOpenGL(LUMEN_WINDOW->GetWindow(), true);
			ImGui_ImplOpenGL3_Init("#version 450");
			break;
		}
		case GraphicsAPI::Vulkan:
		{
			ImGui_ImplGlfw_InitForVulkan(LUMEN_WINDOW->GetWindow(), true);
			ImGui_ImplVulkan_InitInfo init_info = {};
			/*init_info.Instance = g_Instance;
			init_info.PhysicalDevice = g_PhysicalDevice;
			init_info.Device = g_Device;
			init_info.QueueFamily = g_QueueFamily;
			init_info.Queue = g_Queue;
			init_info.PipelineCache = g_PipelineCache;
			init_info.DescriptorPool = g_DescriptorPool;
			init_info.RenderPass = wd->RenderPass;
			init_info.Subpass = 0;
			init_info.MinImageCount = g_MinImageCount;
			init_info.ImageCount = wd->ImageCount;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			init_info.Allocator = g_Allocator;
			init_info.CheckVkResultFn = check_vk_result;*/
			ImGui_ImplVulkan_Init(&init_info);
			break;
		}
		}
	}

	void IMGUIRenderer::ShutDown()
	{
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::OpenGL:
		{
			ImGui_ImplOpenGL3_Shutdown();
			break;
		}
		case GraphicsAPI::Vulkan:
		{
			break;
		}
		}
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void IMGUIRenderer::Begin()
	{
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::OpenGL:
		{
			ImGui_ImplOpenGL3_NewFrame();
			break;
		}
		case GraphicsAPI::Vulkan:
		{
			break;
		}
		}
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void IMGUIRenderer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(
			(float)LUMEN_WINDOW->GetWidth(),
			(float)LUMEN_WINDOW->GetHeight());

		// Rendering
		ImGui::Render();
		switch (LUMEN_GRAPHICS_API)
		{
		case GraphicsAPI::OpenGL:
		{
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			break;
		}
		case GraphicsAPI::Vulkan:
		{
			break;
		}
		}
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	uint32_t IMGUIRenderer::s_ContextID = 0;
	uint32_t IMGUIRenderer::s_Counter;
	std::array<char, 16> IMGUIRenderer::s_IDBuffer;

	void IMGUIRenderer::PushID()
	{
		ImGui::PushID(s_ContextID++);
		s_Counter = 0;
	}

	void IMGUIRenderer::PopID()
	{
		s_ContextID--;
		ImGui::PopID();
	}

	void IMGUIRenderer::BeginPropertyGrid()
	{
		PushID();
		ImGui::Columns(2);
	}

	void IMGUIRenderer::EndPropertyGrid()
	{
		ImGui::Columns(1);
		PopID();
	}

	void IMGUIRenderer::BeginProperty(const char* label)
	{
		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer.data() + 2, 0, 14);
		_itoa(s_Counter++, s_IDBuffer.data() + 2, 16);
	}

	void IMGUIRenderer::EndProperty()
	{
		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}

	bool IMGUIRenderer::Property(const char* label, std::string& value, bool error)
	{
		BeginProperty(label);

		bool modified = false;

		char buffer[256];
		strcpy_s(buffer, value.c_str());

		if (error)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.2f, 1.f));
		if (ImGui::InputText(s_IDBuffer.data(), buffer, 256/*, ImGuiInputTextFlags_EnterReturnsTrue*/))
		{
			value = buffer;
			modified = true;
		}
		if (error)
			ImGui::PopStyleColor();

		EndProperty();

		return modified;
	}

	void IMGUIRenderer::Property(const char* label, const char* value)
	{
		BeginProperty(label);

		ImGui::InputText(s_IDBuffer.data(), (char*)value, 256, ImGuiInputTextFlags_ReadOnly);

		EndProperty();
	}

	bool IMGUIRenderer::Property(const char* label, bool& value)
	{
		BeginProperty(label);

		bool modified = false;
		if (ImGui::Checkbox(s_IDBuffer.data(), &value))
			modified = true;

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::Property(const char* label, int& value)
	{
		BeginProperty(label);

		bool modified = false;
		if (ImGui::DragInt(s_IDBuffer.data(), &value))
			modified = true;

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::PropertySlider(const char* label, int& value, int min, int max)
	{
		BeginProperty(label);

		bool modified = false;
		if (ImGui::SliderInt(s_IDBuffer.data(), &value, min, max))
			modified = true;

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::PropertySlider(const char* label, float& value, float min, float max)
	{
		BeginProperty(label);

		bool modified = false;
		if (ImGui::SliderFloat(s_IDBuffer.data(), &value, min, max))
			modified = true;

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::PropertySlider(const char* label, glm::vec2& value, float min, float max)
	{
		BeginProperty(label);

		bool modified = false;
		if (ImGui::SliderFloat2(s_IDBuffer.data(), glm::value_ptr(value), min, max))
			modified = true;

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::PropertySlider(const char* label, glm::vec3& value, float min, float max)
	{
		BeginProperty(label);

		bool modified = false;
		if (ImGui::SliderFloat3(s_IDBuffer.data(), glm::value_ptr(value), min, max))
			modified = true;

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::PropertySlider(const char* label, glm::vec4& value, float min, float max)
	{
		BeginProperty(label);

		bool modified = false;
		if (ImGui::SliderFloat4(s_IDBuffer.data(), glm::value_ptr(value), min, max))
			modified = true;

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::Property(const char* label, float& value, float delta, float min, float max, bool readOnly)
	{
		BeginProperty(label);

		bool modified = false;
		if (!readOnly)
		{
			if (ImGui::DragFloat(s_IDBuffer.data(), &value, delta, min, max))
				modified = true;
		}
		else
		{
			ImGui::InputFloat(s_IDBuffer.data(), &value, 0.f, 0.f, "%.3f", ImGuiInputTextFlags_ReadOnly);
		}

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::Property(const char* label, glm::vec2& value, float delta)
	{
		BeginProperty(label);

		bool modified = false;
		if (ImGui::DragFloat2(s_IDBuffer.data(), glm::value_ptr(value), delta))
			modified = true;

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::Property(const char* label, glm::vec3& value, float delta)
	{
		BeginProperty(label);

		bool modified = false;
		if (ImGui::DragFloat3(s_IDBuffer.data(), glm::value_ptr(value), delta))
			modified = true;

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::Property(const char* label, glm::vec4& value, float delta)
	{
		BeginProperty(label);

		bool modified = false;
		if (ImGui::DragFloat4(s_IDBuffer.data(), glm::value_ptr(value), delta))
			modified = true;

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::PropertyColor(const char* label, glm::vec3& value)
	{
		BeginProperty(label);

		bool modified = false;
		if (ImGui::ColorEdit3(s_IDBuffer.data(), glm::value_ptr(value)))
			modified = true;

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::PropertyColor(const char* label, glm::vec4& value)
	{
		BeginProperty(label);

		bool modified = false;
		if (ImGui::ColorEdit4(s_IDBuffer.data(), glm::value_ptr(value)))
			modified = true;

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::PropertyDropdown(const char* label, const char* const* options, int32_t optionCount, int32_t* selected)
	{
		BeginProperty(label);

		bool modified = false;
		if (ImGui::Combo(s_IDBuffer.data(), selected, options, optionCount))
			modified = true;

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::PropertyAssetReference(const char* label, AssetHandle& asset, AssetType supportedType)
	{
		BeginProperty(label);

		ImGui::PushID(s_IDBuffer.data());

		ImVec2 width(ImGui::GetContentRegionAvail().x, 0);

		bool modified = false;
		if (asset)
		{
			if (LUMEN_ASSET_MANAGER->GetAssetType(asset) != AssetType::Missing)
			{
				std::string assetName = LUMEN_ASSET_MANAGER->GetAssetName(asset).string();
				ImGui::Button(assetName.c_str(), width);
			}
			else
			{
				ImGui::Button("Missing", width);
			}
		}
		else
		{
			ImGui::Button("Null", width);
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				ASSERT(payload->DataSize == sizeof(AssetHandle));
				AssetHandle handle = *(AssetHandle*)payload->Data;
				if (LUMEN_ASSET_MANAGER->GetAssetType(handle) == supportedType)
				{
					asset = handle;
					modified = true;
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			asset = 0;
			modified = true;
		}

		ImGui::PopID();

		EndProperty();

		return modified;
	}

	bool IMGUIRenderer::BeginTreeNode(const char* name, bool defaultOpen)
	{
		ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
		if (defaultOpen)
			treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

		return ImGui::TreeNodeEx(name, treeNodeFlags);
	}

	void IMGUIRenderer::EndTreeNode()
	{
		ImGui::TreePop();
	}

	uint32_t IMGUIRenderer::s_CheckboxCount = 0;

	void IMGUIRenderer::BeginCheckboxGruop(const char* label)
	{
		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);
	}

	bool IMGUIRenderer::PropertyCheckboxGroup(const char* label, bool& value)
	{
		if (++s_CheckboxCount > 1)
			ImGui::SameLine();

		ImGui::Text(label);
		ImGui::SameLine();

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer.data() + 2, 0, 14);
		_itoa(s_Counter++, s_IDBuffer.data() + 2, 16);

		bool modified = false;
		if (ImGui::Checkbox(s_IDBuffer.data(), &value))
			modified = true;

		return modified;
	}

	void IMGUIRenderer::EndCheckboxGruop()
	{
		ImGui::PopItemWidth();
		ImGui::NextColumn();
		s_CheckboxCount = 0;
	}

	void IMGUIRenderer::Image(AssetHandle handle, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
	{
		auto image = LUMEN_ASSET_MANAGER->GetAsset<Texture>(handle);
		auto width = image->GetWidth();
		auto height = image->GetHeight();
		auto label = std::to_string(width) + " * " + std::to_string(height);
		BeginProperty(label.c_str());

		switch (LUMEN_GRAPHICS_API)
		{
		case Lumen::GraphicsAPI::OpenGL:
		{
			ImGui::Image(image->GetRendererID(), size, uv0, uv1, tint_col, border_col);
			break;
		}
		case Lumen::GraphicsAPI::Vulkan:
		{
			break;
		}
		}

		EndProperty();
	}

	bool IMGUIRenderer::ImageButton(AssetHandle image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& bg_col, const ImVec4& tint_col)
	{
		switch (LUMEN_GRAPHICS_API)
		{
		case Lumen::GraphicsAPI::OpenGL:
		{
			auto glImage = LUMEN_ASSET_MANAGER->GetAsset<Texture>(image);
			return ImGui::ImageButton("image", glImage->GetRendererID(), size, uv0, uv1, bg_col, tint_col);
			break;
		}
		case Lumen::GraphicsAPI::Vulkan:
		{
			break;
		}
		}
		return false;
	}
}