#include "runtime/core/FileDialog.h"

#include "runtime/core/Macro.h"

#ifdef LUMEN_PLATFORM_WINDOWS

#include "runtime/function/global/GlobalContext.h"
#include "runtime/function/render/Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <commdlg.h>
#include <ShlObj.h>

namespace Lumen
{
	std::filesystem::path FileDialog::LoadFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(GlobalContext::Instance().GetWindow()->GetWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::filesystem::path();
	}

	std::filesystem::path FileDialog::SaveFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(GlobalContext::Instance().GetWindow()->GetWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		// Sets the default extension by extracting it from the filter
		ofn.lpstrDefExt = strchr(filter, '\0') + 1;

		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::filesystem::path();
	}

	std::filesystem::path FileDialog::SelectDirectory()
	{
		BROWSEINFO bi = { 0 };
		bi.lpszTitle = L"Select a folder"; // 宽字符字符串  
		bi.ulFlags = BIF_USENEWUI; // 使用新用户界面  

		// 选择文件夹  
		PIDLIST_ABSOLUTE pidList = SHBrowseForFolder(&bi);
		if (pidList != nullptr)
		{
			wchar_t path[MAX_PATH]; // 使用宽字符数组  
			if (SHGetPathFromIDListW(pidList, path)) // 使用宽字符版本的函数  
			{
				CoTaskMemFree(pidList); // 释放内存  
				return std::filesystem::path(path); // 返回 std::filesystem::path  
			}
		}
		return std::filesystem::path(); // 返回一个空路径  
	}
}

#endif // LUMEN_PLATFORM_WINDOWS