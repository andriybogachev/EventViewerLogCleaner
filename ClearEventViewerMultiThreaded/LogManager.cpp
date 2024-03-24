#include "LogManager.hpp"
#include <thread>

#pragma comment(lib, "wevtapi.lib")

std::vector<std::wstring> g_ChannelPaths;
std::wstring allLogs;
std::atomic<int> g_ClearedLogsCount(0);
BOOL g_LogsCleared = FALSE;
BOOL g_CancelOperation = FALSE;

void GetChannelPaths() {
    g_ChannelPaths.clear();

    EVT_HANDLE hChannelEnum = EvtOpenChannelEnum(NULL, 0);
    if (NULL == hChannelEnum) {
        return;
    }

    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    LPWSTR pChannelPath = NULL;
    DWORD status;

    while (1) {
        if (!EvtNextChannelPath(hChannelEnum, dwBufferSize, pChannelPath, &dwBufferUsed)) {
            if (ERROR_NO_MORE_ITEMS == (status = GetLastError())) {
                break;
            }
            else if (ERROR_INSUFFICIENT_BUFFER == status) {
                dwBufferSize = dwBufferUsed;
                pChannelPath = (LPWSTR)malloc(dwBufferSize * sizeof(WCHAR));

                if (pChannelPath) {
                    EvtNextChannelPath(hChannelEnum, dwBufferSize, pChannelPath, &dwBufferUsed);
                }
                else {
                    return;
                }
            }

            if (ERROR_SUCCESS != (status = GetLastError())) {
                return;
            }
        }

        g_ChannelPaths.emplace_back(pChannelPath);
        allLogs += pChannelPath;
        allLogs += L"\r\n";
    }

    if (hChannelEnum)
        EvtClose(hChannelEnum);

    if (pChannelPath)
        free(pChannelPath);
}

void ClearLogsInRange(int start, int end) {
    for (int i = start; i < end; ++i) {
        if (g_CancelOperation) {
            break;
        }
        if (!EvtClearLog(NULL, g_ChannelPaths[i].c_str(), NULL, 0));
        ++g_ClearedLogsCount;
    }
    if (g_ClearedLogsCount > g_ChannelPaths.size()) {
        g_ClearedLogsCount = g_ChannelPaths.size();
    }
}

void ClearAllEventLogs() {
    int totalLogs = g_ChannelPaths.size();
    int numThreads = std::thread::hardware_concurrency();
    int logsPerThread = totalLogs / numThreads;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        int start = i * logsPerThread;
        int end = (i == numThreads - 1) ? totalLogs + 1 : (i + 1) * logsPerThread;
        threads.emplace_back(ClearLogsInRange, start, end);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    g_LogsCleared = TRUE;
}