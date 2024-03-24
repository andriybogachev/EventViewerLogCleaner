#pragma once
#include <windows.h>
#include <winevt.h>
#include <string>
#include <vector>
#include <atomic>

extern std::vector<std::wstring> g_ChannelPaths;
extern std::wstring allLogs;
extern std::atomic<int> g_ClearedLogsCount;
extern BOOL g_LogsCleared;
extern BOOL g_CancelOperation;

void GetChannelPaths();
void ClearLogsInRange(int start, int end);
void ClearAllEventLogs();