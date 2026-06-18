#pragma once

#include "runtime/core/Memory.h"
#include "runtime/resource/asset/Asset.h"

#include <imgui.h>
#include <glm/glm.hpp>

#include <string>
#include <array>
#include <vector>
#include <stack>

namespace Lumen
{
	class IMGUIRenderer
	{
	public:
		static void StartUp();
		static void ShutDown();

		static void Begin();
		static void End();

	public:
		static uint32_t s_ContextID;
		static uint32_t s_Counter;
		static std::array<char, 16> s_IDBuffer;
		static uint32_t s_CheckboxCount;

		static void PushID();
		static void PopID();

		static void BeginPropertyGrid();
		static void EndPropertyGrid();

		static bool Property(const char* label, std::string& value, bool error = false);
		static void Property(const char* label, const char* value);
		static bool Property(const char* label, bool& value);
		static bool Property(const char* label, int& value);
		static bool Property(const char* label, float& value, float delta = 0.1f, float min = 0.0f, float max = 0.0f, bool readOnly = false);
		static bool Property(const char* label, glm::vec2& value, float delta = 0.1f);
		static bool Property(const char* label, glm::vec3& value, float delta = 0.1f);
		static bool Property(const char* label, glm::vec4& value, float delta = 0.1f);

		static bool PropertySlider(const char* label, int& value, int min, int max);
		static bool PropertySlider(const char* label, float& value, float min, float max);
		static bool PropertySlider(const char* label, glm::vec2& value, float min, float max);
		static bool PropertySlider(const char* label, glm::vec3& value, float min, float max);
		static bool PropertySlider(const char* label, glm::vec4& value, float min, float max);
		
		static bool PropertyColor(const char* label, glm::vec3& value);
		static bool PropertyColor(const char* label, glm::vec4& value);
		
		static bool PropertyDropdown(const char* label, const char* const* options, int32_t optionCount, int32_t* selected);
		
		static bool PropertyAssetReference(const char* label, AssetHandle& asset, AssetType supportedType);

		static bool BeginTreeNode(const char* name, bool defaultOpen = true);
		static void EndTreeNode();

		static void BeginCheckboxGruop(const char* label);
		static bool PropertyCheckboxGroup(const char* label, bool& value);
		static void EndCheckboxGruop();

		static void Image(AssetHandle handle, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
		static bool ImageButton(AssetHandle image, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));

	private:
		static void BeginProperty(const char* label);
		static void EndProperty();

	};
}