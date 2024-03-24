#include <windows.h>
#include "LogManager.hpp"
#include "UiUtils.hpp"
#include "resource.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!IsRunAsAdmin())
	{
		AttemptRunAsAdmin();
		return 0;
	}

	// Create window
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = L"WindowClass";
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Window registration failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_TOPMOST, L"WindowClass", L"Clear Event Logs", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0, 500, 450, NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		MessageBox(NULL, L"Window creation failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	if (hIcon)
	{
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	}

	CenterWindow(hwnd);

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	InitializeUI(hwnd, hInstance);

	// Message loop
	MSG Msg;
	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}