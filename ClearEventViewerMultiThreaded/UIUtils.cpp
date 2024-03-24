#include "UiUtils.hpp"
#include "LogManager.hpp"
#include <commctrl.h>
#include <iomanip>
#include <sstream>
#include <thread>

#pragma comment(lib, "comctl32.lib")

HWND hwndPB;
HWND hwndButtonQuit;
HWND hwndEdit;
HWND hwndButtonStart;
HWND hwndButtonCancel;
HWND hwndProgressText;
HWND hwndTotalLogs;
HWND hwndProgressTimer;
DWORD g_dwStartTime = 0;
DWORD g_totalLogs = 0;

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
            ResetUIAfterStart();

            g_dwStartTime = GetTickCount();
            SetTimer(hwnd, 1, 50, NULL);

            GetChannelPaths();

            std::thread progressThread(UpdateProgress);

            ClearAllEventLogs();

            progressThread.detach();
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
            UpdateElapsedTime();

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
        g_LogsCleared = TRUE;
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        g_LogsCleared = TRUE;
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
    SendMessage(hwndPB, PBM_SETBARCOLOR, 0, RGB(0, 200, 0));

    // Create text field
    hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY, 15, 80, 450, 220, hwnd, NULL, hInstance, NULL);
    SendMessage(hwndEdit, EM_SETLIMITTEXT, (WPARAM)1000000, 0);

    // Create button "Start"
    hwndButtonStart = CreateWindowEx(0, WC_BUTTON, L"Start", WS_CHILD | WS_VISIBLE, 100, 310, 120, 40, hwnd, NULL, hInstance, NULL);

    // Create button "Cancel"
    hwndButtonCancel = CreateWindowEx(0, WC_BUTTON, L"Cancel", WS_CHILD | WS_VISIBLE | WS_DISABLED, 260, 310, 120, 40, hwnd, NULL, hInstance, NULL);

    // Create button "Quit"
    hwndButtonQuit = CreateWindowEx(0, WC_BUTTON, L"Quit", WS_CHILD | WS_VISIBLE, 180, 360, 120, 40, hwnd, NULL, hInstance, NULL);
}

void ResetUIAfterStart()
{
    g_ClearedLogsCount = 0;
    g_totalLogs = 0;
    g_dwStartTime = 0;
    g_LogsCleared = FALSE;
    g_CancelOperation = FALSE;

    EnableWindow(hwndButtonStart, FALSE);
    EnableWindow(hwndButtonCancel, TRUE);

    SendMessage(hwndPB, PBM_SETPOS, 0, 0);
    SetWindowText(hwndEdit, L"");
    SetWindowText(hwndProgressText, L"Progress: 0%");
    SetWindowText(hwndTotalLogs, L"Logs: 0");
    SetWindowText(hwndProgressTimer, L"Time 0 sec");
}

void UpdateProgress()
{
    while (true)
    {
        if (!g_LogsCleared)
        {
            int progress = (g_ClearedLogsCount.load() * 100) / g_ChannelPaths.size();
            SendMessage(hwndPB, PBM_SETPOS, (WPARAM)progress, 0);

            std::wstring percentText = L"Progress: " + std::to_wstring(progress) + L"%";
            SetWindowText(hwndProgressText, percentText.c_str());
            SetWindowText(hwndTotalLogs, (L"Logs: " + std::to_wstring(g_ClearedLogsCount.load())).c_str());
        }
        else
        {
            SendMessage(hwndPB, PBM_SETPOS, 100, 0);
            SetWindowText(hwndProgressText, L"Progress: 100%");
            break;
        }
    }
}

void UpdateElapsedTime()
{
    DWORD dwElapsedTime = GetTickCount() - g_dwStartTime;
    float elapsedTimeInSeconds = dwElapsedTime / 1000.0;
    std::wstringstream ss;
    ss << L"Time: " << std::fixed << std::setprecision(3) << elapsedTimeInSeconds << L" sec";
    SetWindowText(hwndProgressTimer, ss.str().c_str());
    SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM)allLogs.c_str());
}