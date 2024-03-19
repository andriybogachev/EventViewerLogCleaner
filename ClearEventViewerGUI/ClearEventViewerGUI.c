#include <windows.h>
#include <commctrl.h>
#include <winevt.h>
#include "resource.h"

#pragma comment(lib, "wevtapi.lib")
#pragma comment(lib, "comctl32.lib")

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

void ClearEventLogCustom(LPCWSTR pwsPath)
{
    if (!EvtClearLog(NULL, pwsPath, NULL, 0))
    {
        // EvtClearLog failed
    }
}

DWORD GetTotalLogsCount()
{
    DWORD totalLogs = 0;
    EVT_HANDLE hChannelEnum = EvtOpenChannelEnum(NULL, 0);
    if (NULL == hChannelEnum)
    {
        return totalLogs;
    }

    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    LPWSTR pChannelPath = NULL;
    DWORD status;

    while (1)
    {
        if (!EvtNextChannelPath(hChannelEnum, dwBufferSize, pChannelPath, &dwBufferUsed))
        {
            if (ERROR_NO_MORE_ITEMS == (status = GetLastError()))
            {
                break;
            }
            else if (ERROR_INSUFFICIENT_BUFFER == status)
            {
                dwBufferSize = dwBufferUsed;
                pChannelPath = (LPWSTR)malloc(dwBufferSize * sizeof(WCHAR));

                if (pChannelPath)
                {
                    EvtNextChannelPath(hChannelEnum, dwBufferSize, pChannelPath, &dwBufferUsed);
                }
                else
                {
                    return totalLogs;
                }
            }

            if (ERROR_SUCCESS != (status = GetLastError()))
            {
                return totalLogs;
            }
        }

        totalLogs++;
    }

    if (hChannelEnum)
        EvtClose(hChannelEnum);

    if (pChannelPath)
        free(pChannelPath);

    return totalLogs;
}

void ClearAllEventLogs(HWND hwndPB)
{
    EVT_HANDLE hChannelEnum = EvtOpenChannelEnum(NULL, 0);
    if (NULL == hChannelEnum)
    {
        return;
    }

    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    LPWSTR pChannelPath = NULL;
    DWORD status;

    int totalLogs = GetTotalLogsCount();

    int i = 0;
    while (1)
    {
        if (!EvtNextChannelPath(hChannelEnum, dwBufferSize, pChannelPath, &dwBufferUsed))
        {
            if (ERROR_NO_MORE_ITEMS == (status = GetLastError()))
            {
                break;
            }
            else if (ERROR_INSUFFICIENT_BUFFER == status)
            {
                dwBufferSize = dwBufferUsed;
                pChannelPath = (LPWSTR)malloc(dwBufferSize * sizeof(WCHAR));

                if (pChannelPath)
                {
                    EvtNextChannelPath(hChannelEnum, dwBufferSize, pChannelPath, &dwBufferUsed);
                }
                else
                {
                    return;
                }
            }

            if (ERROR_SUCCESS != (status = GetLastError()))
            {
                return;
            }
        }

        // Clear the event log
        ClearEventLogCustom(pChannelPath);

        // Update progress bar
        int progress = (i * 100) / totalLogs;
        SendMessage(hwndPB, PBM_SETPOS, (WPARAM)progress, 0);
        SendMessage(hwndPB, PBM_SETBARCOLOR, 0, RGB(0, 200, 0));
        i++;
    }

    if (hChannelEnum)
        EvtClose(hChannelEnum);

    if (pChannelPath)
        free(pChannelPath);
}

HWND hwndPB; // Global variable for the progress bar
HWND hwndButton; // Global variable for the button

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        if ((HWND)lParam == hwndButton)
        {
            PostQuitMessage(0);
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

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
    wc.lpszClassName = "WindowClass";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window registration failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "WindowClass", L"Clear Event Logs", WS_OVERLAPPEDWINDOW, 0, 0, 300, 160, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    if (hIcon)
    {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

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

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Create progress bar
    hwndPB = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE, 15, 20, 250, 30, hwnd, NULL, hInstance, NULL);

    // Create button "OK"
    hwndButton = CreateWindowEx(0, WC_BUTTON, L"OK", WS_CHILD | WS_VISIBLE | WS_DISABLED, 90, 65, 100, 40, hwnd, NULL, hInstance, NULL);

    // Clear event logs
    ClearAllEventLogs(hwndPB);

    // Enable "OK" button
    EnableWindow(hwndButton, TRUE);

    // Message loop
    MSG Msg;
    while (GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    return Msg.wParam;
}
