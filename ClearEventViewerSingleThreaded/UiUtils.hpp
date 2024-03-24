#pragma once
#include <windows.h>

extern HWND hwndButtonQuit;
extern HWND hwndButtonStart;
extern HWND hwndButtonCancel;

BOOL IsRunAsAdmin();
void AttemptRunAsAdmin();
void InitializeUI(HWND hwnd, HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CenterWindow(HWND hwnd);