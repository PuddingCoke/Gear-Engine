#pragma once

#ifndef _GEAR_WINDOW_WIN32FORM_H_
#define _GEAR_WINDOW_WIN32FORM_H_

#define NOMINMAX

#include<Windows.h>

#include<string>

namespace Gear::Window::Win32Form
{
	void initialize(const std::wstring& title, const uint32_t startX, const uint32_t startY, const uint32_t width, const uint32_t height, const DWORD windowStyle,
		LRESULT(*windowCallback)(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam));

	void release();

	bool pollEvents();

	HWND getHandle();

	constexpr DWORD normalWindowStyle = WS_CAPTION | WS_SYSMENU;

	constexpr DWORD wallpaperWindowStyle = WS_POPUP;

	LRESULT CALLBACK windowCallback(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT CALLBACK encodeCallback(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT CALLBACK wallpaperCallBack(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);
}

#endif // !_GEAR_WINDOW_WIN32FORM_H_