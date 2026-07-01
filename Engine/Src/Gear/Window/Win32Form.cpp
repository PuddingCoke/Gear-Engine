#include<Gear/Window/Win32Form.h>

#include<Gear/Utils/Logger.h>

#include<Gear/Core/Graphics.h> 

#include<Gear/Input/Keyboard.h>

#include<Gear/Input/Internal/KeyboardInternal.h>

#include<Gear/Input/Internal/MouseInternal.h>

#include<ImGUI/imgui.h>

#define WM_TRAYICON WM_USER

#define EXITUID 0

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);

namespace Gear::Window::Win32Form
{
	class Win32FormImpl
	{
	public:

		Win32FormImpl() = delete;

		Win32FormImpl(const Win32FormImpl&) = delete;

		void operator=(const Win32FormImpl&) = delete;

		Win32FormImpl(const std::wstring& title, const uint32_t startX, const uint32_t startY, const uint32_t width, const uint32_t height, const DWORD windowStyle,
			LRESULT(*windowCallback)(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam));

		~Win32FormImpl();

		bool pollEvents();

		bool pollEvents(const DWORD millisecond);

		HWND getHandle() const;

		LRESULT CALLBACK windowProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam) const;

		LRESULT CALLBACK encodeProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam) const;

		LRESULT CALLBACK wallpaperProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam) const;

		LRESULT CALLBACK mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) const;

	private:

		HWND hWnd;

		const bool iniTrayIcon;

		HMENU hMenu;

		NOTIFYICONDATA nid;

		HHOOK mouseHook;

	};

	Win32FormImpl::Win32FormImpl(const std::wstring& title, const uint32_t startX, const uint32_t startY, const uint32_t width, const uint32_t height, const DWORD windowStyle, LRESULT(*windowCallback)(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam)) :
		hWnd(nullptr), iniTrayIcon(windowCallback == wallpaperCallBack), hMenu(nullptr), nid{}, mouseHook(nullptr)
	{
		SetProcessDPIAware();

		const HINSTANCE hInstance = GetModuleHandle(0);

		WNDCLASSEX wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
		wcex.hIcon = LoadIcon(0, IDI_APPLICATION);
		wcex.hIconSm = LoadIcon(0, IDI_APPLICATION);
		wcex.lpszClassName = L"MyWindowClass";
		wcex.hInstance = hInstance;
		wcex.lpfnWndProc = windowCallback;

		RegisterClassEx(&wcex);

		RECT rect = { 0,0,static_cast<LONG>(width),static_cast<LONG>(height) };

		AdjustWindowRect(&rect, windowStyle, false);

		hWnd = CreateWindow(L"MyWindowClass", title.c_str(), windowStyle, startX, startY,
			rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);

		if (!hWnd)
		{
			LOGERROR(L"创建窗体失败！");
		}

		ShowWindow(hWnd, SW_SHOW);

		if (iniTrayIcon)
		{
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = hWnd;
			nid.uID = 0;
			nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
			nid.uCallbackMessage = WM_TRAYICON;
			nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
			wcscpy_s(nid.szTip, L"动态壁纸");

			Shell_NotifyIcon(NIM_ADD, &nid);

			hMenu = CreatePopupMenu();

			AppendMenu(hMenu, MF_STRING, EXITUID, L"退出程序");

			mouseHook = SetWindowsHookEx(WH_MOUSE_LL, Win32Form::mouseHookProc, nullptr, 0);
		}
	}

	Win32FormImpl::~Win32FormImpl()
	{
		if (iniTrayIcon)
		{
			UnhookWindowsHookEx(mouseHook);

			DestroyMenu(hMenu);

			Shell_NotifyIcon(NIM_DELETE, &nid);
		}

		DestroyWindow(hWnd);
	}

	bool Win32FormImpl::pollEvents()
	{
		Input::Mouse::Internal::resetDeltaValue();

		Input::Keyboard::Internal::resetDeltaValue();

		MSG msg = {};

		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);

			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				return false;
			}
		}

		return true;
	}

	bool Win32FormImpl::pollEvents(const DWORD millisecond)
	{
		const DWORD result = MsgWaitForMultipleObjectsEx(0, nullptr, millisecond, QS_POSTMESSAGE | QS_SENDMESSAGE, 0);

		if (result == WAIT_OBJECT_0)
		{
			MSG msg;

			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);

				DispatchMessage(&msg);

				if (msg.message == WM_QUIT)
				{
					return false;
				}
			}
		}

		return true;
	}

	HWND Win32FormImpl::getHandle() const
	{
		return hWnd;
	}

	LRESULT Win32FormImpl::windowProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam) const
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;

		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;

			BeginPaint(hWnd, &ps);

			EndPaint(hWnd, &ps);
		}
		break;

		case WM_MOUSEMOVE:
			if (!ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureMouse)
			{
				Input::Mouse::Internal::move(static_cast<float>(LOWORD(lParam)), static_cast<float>(Core::Graphics::getHeight()) - static_cast<float>(HIWORD(lParam)));
			}
			break;

		case WM_LBUTTONDOWN:
			if (!ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureMouse)
			{
				Input::Mouse::Internal::pressLeft();
			}
			break;

		case WM_RBUTTONDOWN:
			if (!ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureMouse)
			{
				Input::Mouse::Internal::pressRight();
			}
			break;

		case WM_LBUTTONUP:
			if (!ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureMouse)
			{
				Input::Mouse::Internal::releaseLeft();
			}
			break;

		case WM_RBUTTONUP:
			if (!ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureMouse)
			{
				Input::Mouse::Internal::releaseRight();
			}
			break;

		case WM_MOUSEWHEEL:
			if (!ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureMouse)
			{
				Input::Mouse::Internal::scroll(GET_WHEEL_DELTA_WPARAM(wParam) / 120.f);
			}
			break;

		case WM_KEYDOWN:
			if (((HIWORD(lParam) & KF_REPEAT) == 0) && (!ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureKeyboard))
			{
				Input::Keyboard::Internal::pressKey(static_cast<Input::Keyboard::Key>(wParam));
			}
			break;

		case WM_KEYUP:
			if (!ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureKeyboard)
			{
				Input::Keyboard::Internal::releaseKey(static_cast<Input::Keyboard::Key>(wParam));
			}
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		return 0;
	}

	LRESULT Win32FormImpl::encodeProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam) const
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;

			BeginPaint(hWnd, &ps);

			EndPaint(hWnd, &ps);
		}
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		return 0;
	}

	LRESULT Win32FormImpl::wallpaperProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam) const
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;

			BeginPaint(hWnd, &ps);

			EndPaint(hWnd, &ps);
		}
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_TRAYICON:
			if (LOWORD(lParam) == WM_RBUTTONUP)
			{
				POINT pt;

				GetCursorPos(&pt);

				SetForegroundWindow(hWnd);

				TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
			}
			break;

		case WM_COMMAND:
			if (LOWORD(wParam) == EXITUID)
			{
				PostQuitMessage(0);
			}
			break;

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		return 0;
	}

	LRESULT Win32FormImpl::mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) const
	{
		if (nCode == HC_ACTION)
		{
			const MSLLHOOKSTRUCT* const pMouseStruct = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);

			switch (wParam)
			{
			case WM_MOUSEMOVE:
				Input::Mouse::Internal::move(static_cast<float>(pMouseStruct->pt.x), static_cast<float>(Core::Graphics::getHeight()) - static_cast<float>(pMouseStruct->pt.y));
				break;

			case WM_LBUTTONDOWN:
				Input::Mouse::Internal::pressLeft();
				break;

			case WM_RBUTTONDOWN:
				Input::Mouse::Internal::pressRight();
				break;

			case WM_LBUTTONUP:
				Input::Mouse::Internal::releaseLeft();
				break;

			case WM_RBUTTONUP:
				Input::Mouse::Internal::releaseRight();
				break;
			}

		}

		return CallNextHookEx(mouseHook, nCode, wParam, lParam);
	}

	UniquePtr<Win32FormImpl> impl;

	void initialize(const std::wstring& title, const uint32_t startX, const uint32_t startY, const uint32_t width, const uint32_t height, const DWORD windowStyle, LRESULT(*windowCallback)(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam))
	{
		impl = makeUnique<Win32FormImpl>(title, startX, startY, width, height, windowStyle, windowCallback);
	}

	void release()
	{
		impl.reset();
	}

	bool pollEvents()
	{
		return impl->pollEvents();
	}

	bool pollEvents(const DWORD millisecond)
	{
		return impl->pollEvents(millisecond);
	}

	HWND getHandle()
	{
		return impl->getHandle();
	}

	LRESULT CALLBACK windowCallback(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
	{
		return impl->windowProc(hWnd, uMsg, wParam, lParam);
	}

	LRESULT CALLBACK encodeCallback(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
	{
		return impl->encodeProc(hWnd, uMsg, wParam, lParam);
	}

	LRESULT CALLBACK wallpaperCallBack(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
	{
		return impl->wallpaperProc(hWnd, uMsg, wParam, lParam);
	}

	LRESULT mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		return impl->mouseHookProc(nCode, wParam, lParam);
	}
}