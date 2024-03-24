#pragma once
#include <windows.h>

extern HWND hwndPB;
extern HWND hwndEdit;
extern HWND hwndProgressText;
extern HWND hwndTotalLogs;
extern HWND hwndProgressTimer;
extern DWORD g_dwStartTime;
extern BOOL g_CancelOperation;
extern BOOL g_LogsCleared;
extern int numThreads;

DWORD GetTotalLogsCount();
void ClearAllEventLogs(HWND hwndPB);
void ClearAllEventLogsThread(HWND hwndPB);
void UpdateElapsedTime(HWND hwndProgressTimer);