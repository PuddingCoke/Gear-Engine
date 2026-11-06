#include <Gear/Window/Win32Form.h>

#include <Gear/Utils/Log.h>

#include <Gear/Utils/WallpaperHelper.h>

#include <Gear/Core/Graphics.h>

#include <Gear/Input/Keyboard.h>

#include <Gear/Input/Internal/InputInternal.h>

#include <Gear/Input/Internal/KeyboardInternal.h>

#include <Gear/Input/Internal/MouseInternal.h>

#include <ImGUI/imgui.h>

#include <shellapi.h>

#include <hidusage.h>

#define WM_TRAYICON WM_USER

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             uint32_t uMsg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

namespace Gear::Window::Win32Form
{
	LRESULT CALLBACK menuWindowCallBack(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	class Win32FormImpl
	{
	public:

		Win32FormImpl() = delete;

		Win32FormImpl(const Win32FormImpl&) = delete;

		void operator=(const Win32FormImpl&) = delete;

		Win32FormImpl(const std::wstring& title,
		              const uint32_t startX,
		              const uint32_t startY,
		              const uint32_t width,
		              const uint32_t height,
		              const DWORD windowStyle,
		              LRESULT (*windowCallback)(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam));

		~Win32FormImpl();

		bool pollEvents(const DWORD milliseconds);

		HWND getHandle() const;

		LRESULT CALLBACK realTimeRenderProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);

		LRESULT CALLBACK videoRenderProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);

		LRESULT CALLBACK wallpaperProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);

		LRESULT CALLBACK menuWindowProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);

	private:

		static constexpr int32_t menuWidth = 80;

		static constexpr int32_t menuHeight = 40;

		HWND windowHandle;

		HWND menuWindowHandle;

		const bool initTrayIcon;

		NOTIFYICONDATA trayIconData;

		RAWINPUTDEVICE rawInputMouse;

		RAWINPUTDEVICE rawInputKeyboard;
	};

	Win32FormImpl::Win32FormImpl(
		const std::wstring& title,
		const uint32_t startX,
		const uint32_t startY,
		const uint32_t width,
		const uint32_t height,
		const DWORD windowStyle,
		LRESULT (*windowCallback)(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam)) :
		windowHandle(nullptr),
		menuWindowHandle(nullptr),
		initTrayIcon(windowCallback == wallpaperCallBack),
		trayIconData{},
		rawInputMouse{},
		rawInputKeyboard{}
	{
		//传入的width、height是像素尺度
		//因此不能让窗口被自动缩放
		SetProcessDPIAware();

		const HINSTANCE hInstance = GetModuleHandle(0);

		{
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
		}

		RECT rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

		AdjustWindowRect(&rect, windowStyle, false);

		windowHandle = CreateWindow(L"MyWindowClass",
		                            title.c_str(),
		                            windowStyle,
		                            startX,
		                            startY,
		                            rect.right - rect.left,
		                            rect.bottom - rect.top,
		                            nullptr,
		                            nullptr,
		                            hInstance,
		                            nullptr);

		if (!windowHandle)
		{
			THROWLOG(LOGERROR() << "主窗口创建失败");
		}

		ShowWindow(windowHandle, SW_SHOW);

		{
			rawInputMouse.usUsagePage = HID_USAGE_PAGE_GENERIC;
			rawInputMouse.usUsage = HID_USAGE_GENERIC_MOUSE;
			rawInputMouse.dwFlags = initTrayIcon ? RIDEV_EXINPUTSINK : 0;
			rawInputMouse.hwndTarget = windowHandle;

			if (!RegisterRawInputDevices(&rawInputMouse, 1, sizeof(rawInputMouse)))
			{
				THROWLOG(LOGERROR() << TOSTRING(RegisterRawInputDevices) << "调用失败，失败值"
				                    << LogIntegerMode::HEX << static_cast<uint32_t>(GetLastError()));
			}
		}

		{
			rawInputKeyboard.usUsagePage = HID_USAGE_PAGE_GENERIC;
			rawInputKeyboard.usUsage = HID_USAGE_GENERIC_KEYBOARD;
			rawInputKeyboard.dwFlags = initTrayIcon ? RIDEV_EXINPUTSINK : 0;
			rawInputKeyboard.hwndTarget = windowHandle;

			if (!RegisterRawInputDevices(&rawInputKeyboard, 1, sizeof(rawInputKeyboard)))
			{
				THROWLOG(LOGERROR() << TOSTRING(RegisterRawInputDevices) << "调用失败，失败值"
				                    << LogIntegerMode::HEX << static_cast<uint32_t>(GetLastError()));
			}
		}

		if (initTrayIcon)
		{
			{
				WNDCLASSEX wcex = {};
				wcex.cbSize = sizeof(WNDCLASSEX);
				wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
				wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
				wcex.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
				wcex.lpszClassName = L"MenuWindowClass";
				wcex.hInstance = hInstance;
				wcex.lpfnWndProc = menuWindowCallBack;
				RegisterClassEx(&wcex);
			}

			menuWindowHandle = CreateWindowEx(WS_EX_TOOLWINDOW,
			                                  L"MenuWindowClass",
			                                  L"",
			                                  WS_POPUP | WS_BORDER,
			                                  0,
			                                  0,
			                                  menuWidth,
			                                  menuHeight,
			                                  windowHandle,
			                                  nullptr,
			                                  GetModuleHandle(0),
			                                  nullptr);

			if (!menuWindowHandle)
			{
				THROWLOG(LOGERROR() << "菜单窗口创建失败");
			}

			trayIconData.cbSize = sizeof(NOTIFYICONDATA);
			trayIconData.hWnd = menuWindowHandle;
			trayIconData.uID = 0;
			trayIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
			trayIconData.uCallbackMessage = WM_TRAYICON;
			trayIconData.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
			wcscpy_s(trayIconData.szTip, L"动态壁纸");

			if (!Shell_NotifyIcon(NIM_ADD, &trayIconData))
			{
				THROWLOG(LOGERROR() << "托盘图标创建失败");
			}
		}
	}

	Win32FormImpl::~Win32FormImpl()
	{
		if (initTrayIcon)
		{
			Shell_NotifyIcon(NIM_DELETE, &trayIconData);

			if (menuWindowHandle)
			{
				DestroyWindow(menuWindowHandle);
			}
		}

		{
			rawInputMouse.dwFlags = RIDEV_REMOVE;
			rawInputMouse.hwndTarget = nullptr;

			RegisterRawInputDevices(&rawInputMouse, 1, sizeof(rawInputMouse));
		}

		{
			rawInputKeyboard.dwFlags = RIDEV_REMOVE;
			rawInputKeyboard.hwndTarget = nullptr;

			RegisterRawInputDevices(&rawInputKeyboard, 1, sizeof(rawInputKeyboard));
		}

		if (windowHandle)
		{
			DestroyWindow(windowHandle);
		}
	}

	bool Win32FormImpl::pollEvents(const DWORD milliseconds)
	{
		Input::Internal::clearEventTriggerList();

		Input::Mouse::Internal::resetDeltaValue();

		Input::Keyboard::Internal::resetDeltaValue();

		MSG msg;

		const DWORD result = MsgWaitForMultipleObjectsEx(0, nullptr, milliseconds, QS_ALLINPUT, 0);

		if (result == WAIT_OBJECT_0)
		{
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

		Input::Internal::triggerEvents();

		return true;
	}

	HWND Win32FormImpl::getHandle() const
	{
		return windowHandle;
	}

	LRESULT Win32FormImpl::realTimeRenderProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
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
				Input::Mouse::Internal::setPosition(static_cast<float>(LOWORD(lParam)),
				                                    static_cast<float>(Core::Graphics::getHeight()) -
				                                        static_cast<float>(HIWORD(lParam)));
			}

			break;

		case WM_INPUT:
		{
			uint32_t dataSize = 0;

			if (!GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
			                     RID_INPUT,
			                     nullptr,
			                     &dataSize,
			                     sizeof(RAWINPUTHEADER)) &&
			    dataSize <= sizeof(RAWINPUT))
			{
				RAWINPUT raw;

				if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
				                    RID_INPUT,
				                    &raw,
				                    &dataSize,
				                    sizeof(RAWINPUTHEADER)) == dataSize)
				{
					if (raw.header.dwType == RIM_TYPEMOUSE)
					{
						const RAWMOUSE& mouse = raw.data.mouse;

						if (!ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureMouse)
						{
							if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
							{
								Input::Mouse::Internal::pressLeft();
							}

							if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
							{
								Input::Mouse::Internal::releaseLeft();
							}

							if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
							{
								Input::Mouse::Internal::pressRight();
							}

							if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
							{
								Input::Mouse::Internal::releaseRight();
							}

							if (mouse.usButtonFlags & RI_MOUSE_WHEEL)
							{
								const float delta =
									static_cast<float>(static_cast<SHORT>(mouse.usButtonData)) /
									static_cast<float>(WHEEL_DELTA);

								Input::Mouse::Internal::scroll(delta);
							}

							if ((mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_RELATIVE)
							{
								Input::Mouse::Internal::move(static_cast<float>(mouse.lLastX),
								                             static_cast<float>(-mouse.lLastY));
							}
						}
					}
					else if (raw.header.dwType == RIM_TYPEKEYBOARD)
					{
						const RAWKEYBOARD& keyboard = raw.data.keyboard;

						if (!ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureKeyboard)
						{
							const Input::Keyboard::Key key = static_cast<Input::Keyboard::Key>(keyboard.VKey);

							if ((keyboard.Flags & RI_KEY_BREAK) == 0)
							{
								//不处理持续触发的情况
								if (!Input::Keyboard::getKeyDown(key))
								{
									Input::Keyboard::Internal::pressKey(key);
								}
							}
							else
							{
								Input::Keyboard::Internal::releaseKey(key);
							}
						}
					}
				}
			}
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

	LRESULT Win32FormImpl::videoRenderProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
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

	LRESULT Win32FormImpl::wallpaperProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
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

		case WM_INPUT:
		{
			uint32_t dataSize = 0;

			if (!GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
			                     RID_INPUT,
			                     nullptr,
			                     &dataSize,
			                     sizeof(RAWINPUTHEADER)) &&
			    dataSize <= sizeof(RAWINPUT))
			{
				RAWINPUT raw;

				if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
				                    RID_INPUT,
				                    &raw,
				                    &dataSize,
				                    sizeof(RAWINPUTHEADER)) == dataSize)
				{
					if (raw.header.dwType == RIM_TYPEMOUSE)
					{
						if (Utils::WallpaperHelper::isOnDesktop(GetForegroundWindow()))
						{
							const RAWMOUSE& mouse = raw.data.mouse;

							if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
							{
								Input::Mouse::Internal::pressLeft();
							}

							if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
							{
								Input::Mouse::Internal::releaseLeft();
							}

							if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
							{
								Input::Mouse::Internal::pressRight();
							}

							if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
							{
								Input::Mouse::Internal::releaseRight();
							}

							if (mouse.usButtonFlags & RI_MOUSE_WHEEL)
							{
								const float delta =
									static_cast<float>(static_cast<SHORT>(mouse.usButtonData)) /
									static_cast<float>(WHEEL_DELTA);

								Input::Mouse::Internal::scroll(delta);
							}

							if ((mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_RELATIVE)
							{
								POINT pt;

								GetPhysicalCursorPos(&pt);

								Input::Mouse::Internal::setPosition(
									static_cast<float>(pt.x),
									static_cast<float>(Core::Graphics::getHeight()) -
										static_cast<float>(pt.y));

								Input::Mouse::Internal::move(static_cast<float>(mouse.lLastX),
								                             static_cast<float>(-mouse.lLastY));
							}
						}
					}
					else if (raw.header.dwType == RIM_TYPEKEYBOARD)
					{
						if (Utils::WallpaperHelper::isOnDesktop(GetForegroundWindow()))
						{
							const RAWKEYBOARD& keyboard = raw.data.keyboard;

							const Input::Keyboard::Key key = static_cast<Input::Keyboard::Key>(keyboard.VKey);

							if ((keyboard.Flags & RI_KEY_BREAK) == 0)
							{
								//不处理持续触发的情况
								if (!Input::Keyboard::getKeyDown(key))
								{
									Input::Keyboard::Internal::pressKey(key);
								}
							}
							else
							{
								Input::Keyboard::Internal::releaseKey(key);
							}
						}
					}
				}
			}
		}

		break;

		default:

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		return 0;
	}

	LRESULT Win32FormImpl::menuWindowProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;

			HDC hdc = BeginPaint(hWnd, &ps);

			RECT rc;

			GetClientRect(hWnd, &rc);

			SetBkMode(hdc, TRANSPARENT);

			SetTextColor(hdc, RGB(0, 0, 0));

			DrawTextW(hdc, L"退出程序", -1, &rc, DT_VCENTER | DT_CENTER | DT_SINGLELINE);

			EndPaint(hWnd, &ps);

			break;
		}

		case WM_TRAYICON:

			if (LOWORD(lParam) == WM_RBUTTONUP)
			{
				POINT pt;

				GetCursorPos(&pt);

				SetWindowPos(hWnd,
				             HWND_TOPMOST,
				             pt.x,
				             pt.y - menuHeight,
				             menuWidth,
				             menuHeight,
				             SWP_NOACTIVATE | SWP_SHOWWINDOW);

				SetForegroundWindow(hWnd);

				SetCapture(hWnd);
			}

			break;

			//https://cloud.tencent.com/developer/article/2091013
		case WM_LBUTTONUP:
		{
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };

			ClientToScreen(hWnd, &pt);

			RECT rc;

			GetWindowRect(hWnd, &rc);

			if (PtInRect(&rc, pt))
			{
				PostQuitMessage(0);
			}

			ReleaseCapture();

			break;
		}

		case WM_CAPTURECHANGED:

			ShowWindow(hWnd, SW_HIDE);

			break;

		default:

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		return 0;
	}

	UniquePtr<Win32FormImpl> impl;

	void initialize(const std::wstring& title,
	                const uint32_t startX,
	                const uint32_t startY,
	                const uint32_t width,
	                const uint32_t height,
	                const DWORD windowStyle,
	                LRESULT (*windowCallback)(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam))
	{
		impl = makeUnique<Win32FormImpl>(title, startX, startY, width, height, windowStyle, windowCallback);
	}

	void release()
	{
		impl.reset();
	}

	bool pollEvents(const DWORD milliseconds)
	{
		return impl->pollEvents(milliseconds);
	}

	HWND getHandle()
	{
		return impl->getHandle();
	}

	LRESULT CALLBACK realTimeRenderCallback(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
	{
		return impl->realTimeRenderProc(hWnd, uMsg, wParam, lParam);
	}

	LRESULT CALLBACK videoRenderCallback(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
	{
		return impl->videoRenderProc(hWnd, uMsg, wParam, lParam);
	}

	LRESULT CALLBACK wallpaperCallBack(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
	{
		return impl->wallpaperProc(hWnd, uMsg, wParam, lParam);
	}

	LRESULT CALLBACK menuWindowCallBack(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return impl->menuWindowProc(hWnd, uMsg, wParam, lParam);
	}
}
