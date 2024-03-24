#pragma once
#include <windows.h>

extern HWND hwndPB;
extern HWND hwndButtonQuit;
extern HWND hwndEdit;
extern HWND hwndButtonStart;
extern HWND hwndButtonCancel;
extern HWND hwndProgressText;
extern HWND hwndTotalLogs;
extern HWND hwndProgressTimer;
extern DWORD g_dwStartTime;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CenterWindow(HWND hwnd);
void InitializeUI(HWND hwnd, HINSTANCE hInstance);
BOOL IsRunAsAdmin();
void AttemptRunAsAdmin();
void ResetUIAfterStart();
void UpdateProgress();
void UpdateElapsedTime();