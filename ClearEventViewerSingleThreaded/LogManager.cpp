#include "LogManager.hpp"
#include "UiUtils.hpp"
#include <winevt.h>
#include <string>
#include <thread>
#include <sstream>
#include <iomanip>
#include <commctrl.h>

#pragma comment(lib, "wevtapi.lib")

HWND hwndPB;
HWND hwndEdit;
HWND hwndProgressText;
HWND hwndTotalLogs;
HWND hwndProgressTimer;
DWORD g_dwStartTime = 0;
BOOL g_CancelOperation = FALSE;
BOOL g_LogsCleared = FALSE;
int numThreads = std::thread::hardware_concurrency();

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

    int actualLogs = 0;
    while (1)
    {
        if (g_CancelOperation)
        {
            break;
        }
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
        EvtClearLog(NULL, pChannelPath, NULL, 0);

        actualLogs++;

        // Update progress bar
        int progress = (actualLogs * 100) / totalLogs;

        SendMessage(hwndPB, PBM_SETPOS, (WPARAM)progress, 0);
        SendMessage(hwndPB, PBM_SETBARCOLOR, 0, RGB(0, 200, 0));

        std::wstring percentText = L"Progress: " + std::to_wstring(progress) + L"%";
        SetWindowText(hwndProgressText, percentText.c_str());


        const int MAX_CHARACTERS = 1000000;
        SendMessage(hwndEdit, EM_SETLIMITTEXT, (WPARAM)MAX_CHARACTERS, 0);
        SendMessage(hwndEdit, EM_REPLACESEL, FALSE, (LPARAM)(std::wstring(pChannelPath) + L"\r\n").c_str());
        SendMessage(hwndTotalLogs, WM_SETTEXT, 0, reinterpret_cast<LPARAM>((L"Logs: " + std::to_wstring(actualLogs)).c_str()));
    }

    if (hChannelEnum)
        EvtClose(hChannelEnum);

    if (pChannelPath)
        free(pChannelPath);

    g_LogsCleared = TRUE;
    EnableWindow(hwndButtonCancel, FALSE);
    EnableWindow(hwndButtonStart, TRUE);

    SendMessage(hwndEdit, EM_REPLACESEL, FALSE, (LPARAM)std::to_wstring(numThreads).c_str());
}

void ClearAllEventLogsThread(HWND hwndPB)
{
    ClearAllEventLogs(hwndPB);
}

void UpdateElapsedTime(HWND hwndProgressTimer)
{
    DWORD dwElapsedTime = GetTickCount() - g_dwStartTime;
    float elapsedTimeInSeconds = dwElapsedTime / 1000.0;
    std::wstringstream ss;
    ss << L"Time: " << std::fixed << std::setprecision(3) << elapsedTimeInSeconds << L" sec";
    SetWindowText(hwndProgressTimer, ss.str().c_str());
}