#include "menu/menu.h"

#include "imgui/imgui_impl_win32.h"

#include "configManager/settings.h"
#include "base.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef LRESULT(CALLBACK* template_WndProc) (HWND, UINT, WPARAM, LPARAM);
template_WndProc original_wndProc;
LRESULT CALLBACK hook_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	std::unique_lock<std::recursive_mutex> stateLock(Base::StateMutex);
	if (!Base::Running) {
		stateLock.unlock();
		return CallWindowProc(original_wndProc, hwnd, msg, wParam, lParam);
	}
	if (msg == WM_KEYDOWN && !(lParam & (1LL << 30)))
	{
		if (wParam == Menu::Keybind)
		{
			// This callback may run on a different thread than the JNI environment.
			// Do not query Minecraft here.
			Menu::Open = !Menu::Open;
		}
		if (wParam == VK_ESCAPE && Menu::Open)
		{
			Menu::MoveCursorToCenter(false);
			Menu::Open = false;
		}
	}

	if (Menu::Open && Menu::Initialized)
	{
		if (settings::Menu_GUIMovement)
		{
			// block imgui input from reaching minecraft
			if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
				return true;

			// block mouse move events from reaching minecraft
			if (msg == WM_MOUSEMOVE)
				return true;

			// block mouse click events from reaching minecraft
			if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN)
				return true;

			// block mouse scroll events from reaching minecraft
			if (msg == WM_MOUSEWHEEL)
				return true;
		}
		else
		{
			ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);
			if ((msg >= WM_KEYFIRST && msg <= WM_KEYLAST) ||
				(msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)) return true;
		}
	}

	stateLock.unlock();
	return CallWindowProc(original_wndProc, hwnd, msg, wParam, lParam);
}



void Menu::Hook_wndProc()
{
	original_wndProc = (template_WndProc)SetWindowLongPtr(Menu::HandleWindow, GWLP_WNDPROC, (LONG_PTR)hook_WndProc);
}

void Menu::Unhook_wndProc()
{
	SetWindowLongPtr(Menu::HandleWindow, GWLP_WNDPROC, (LONG_PTR)original_wndProc);
}

