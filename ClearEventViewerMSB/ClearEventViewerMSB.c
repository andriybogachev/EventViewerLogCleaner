#include <windows.h>
#include <winevt.h>

#pragma comment(lib, "wevtapi.lib")

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

void ClearAllEventLogs()
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

    }

    if (hChannelEnum)
        EvtClose(hChannelEnum);

    if (pChannelPath)
        free(pChannelPath);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    if (!IsRunAsAdmin())
    {
        AttemptRunAsAdmin();
        return 0;
    }

    // Clear event logs
    ClearAllEventLogs();

    // Display success message
    MessageBox(NULL, L"All event logs have been cleared successfully.", L"Success", MB_ICONINFORMATION | MB_TOPMOST);

    return 0;
}
