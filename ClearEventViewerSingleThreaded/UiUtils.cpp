#include "UiUtils.hpp"
#include "LogManager.hpp"
#include <commctrl.h>
#include <shellapi.h>
#include <thread>

#pragma comment(lib, "comctl32.lib")

HWND hwndButtonQuit;
HWND hwndButtonStart;
HWND hwndButtonCancel;

BOOL IsRunAsAdmin()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

	if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:
	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	if (ERROR_SUCCESS != dwError)
	{
		exit(dwError);
	}

	return fIsRunAsAdmin;
}

void AttemptRunAsAdmin()
{
	WCHAR szPath[MAX_PATH];
	if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
	{
		SHELLEXECUTEINFO sei = { sizeof(sei) };
		sei.lpVerb = L"runas";
		sei.lpFile = szPath;
		sei.hwnd = NULL;
		sei.nShow = SW_NORMAL;

		if (!ShellExecuteEx(&sei))
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_CANCELLED)
			{
				exit(0);
			}
		}
	}
}

void InitializeUI(HWND hwnd, HINSTANCE hInstance)
{
	// Create ProgressText
	hwndProgressText = CreateWindowEx(0, L"STATIC", L"Progress: 0%", WS_CHILD | WS_VISIBLE, 15, 10, 120, 30, hwnd, NULL, hInstance, NULL);

	// Create TotalCount
	hwndTotalLogs = CreateWindowEx(0, L"STATIC", L"Logs: 0", WS_CHILD | WS_VISIBLE, 200, 10, 120, 30, hwnd, NULL, hInstance, NULL);

	// Create Timer
	hwndProgressTimer = CreateWindowEx(0, L"STATIC", L"Time 0 sec", WS_CHILD | WS_VISIBLE, 360, 10, 120, 30, hwnd, NULL, hInstance, NULL);

	// Create progress bar
	hwndPB = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE, 15, 40, 450, 30, hwnd, NULL, hInstance, NULL);

	// Create text field
	hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY, 15, 80, 450, 220, hwnd, NULL, hInstance, NULL);

	// Create button "Start"
	hwndButtonStart = CreateWindowEx(0, WC_BUTTON, L"Start", WS_CHILD | WS_VISIBLE, 100, 310, 120, 40, hwnd, NULL, hInstance, NULL);

	// Create button "Cancel"
	hwndButtonCancel = CreateWindowEx(0, WC_BUTTON, L"Cancel", WS_CHILD | WS_VISIBLE | WS_DISABLED, 260, 310, 120, 40, hwnd, NULL, hInstance, NULL);

	// Create button "Quit"
	hwndButtonQuit = CreateWindowEx(0, WC_BUTTON, L"Quit", WS_CHILD | WS_VISIBLE, 180, 360, 120, 40, hwnd, NULL, hInstance, NULL);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		if ((HWND)lParam == hwndButtonQuit)
		{
			g_CancelOperation = TRUE;
			PostQuitMessage(0);
		}
		else if ((HWND)lParam == hwndButtonStart)
		{
			g_dwStartTime = 0;
			g_LogsCleared = FALSE;
			g_CancelOperation = FALSE;
			EnableWindow(hwndButtonStart, FALSE);
			EnableWindow(hwndButtonCancel, TRUE);
			SendMessage(hwndPB, PBM_SETPOS, 0, 0);
			SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM)L"");

			g_dwStartTime = GetTickCount();
			SetTimer(hwnd, 1, 50, NULL);

			std::thread clearThread(ClearAllEventLogsThread, hwndPB);
			clearThread.detach();
		}
		else if ((HWND)lParam == hwndButtonCancel)
		{
			g_LogsCleared = TRUE;
			g_CancelOperation = TRUE;
			EnableWindow(hwndButtonStart, TRUE);
			EnableWindow(hwndButtonCancel, FALSE);
		}
		break;
	case WM_TIMER:
	{
		if (wParam == 1)
		{
			UpdateElapsedTime(hwndProgressTimer);

			if (g_LogsCleared)
			{
				KillTimer(hwnd, 1);
				EnableWindow(hwndButtonCancel, FALSE);
				EnableWindow(hwndButtonStart, TRUE);
			}
		}
	}
	break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CTLCOLORSTATIC:
		return (INT_PTR)GetStockObject(WHITE_BRUSH);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void CenterWindow(HWND hwnd)
{
	RECT rc;
	GetWindowRect(hwnd, &rc);

	// Get screen dimensions
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Calculate coordinates for centering window
	int posX = (screenWidth - rc.right) / 2;
	int posY = (screenHeight - rc.bottom) / 2;

	// Set new coordinates
	SetWindowPos(hwnd, HWND_TOP, posX, posY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}