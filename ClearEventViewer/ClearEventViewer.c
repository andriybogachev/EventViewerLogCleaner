#include <windows.h>
#include <winevt.h>
#include <stdio.h>

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
                // The user refused the elevation.
                // Do nothing ...
            }
        }
    }
}

void ClearEventLogCustom(LPCWSTR pwsPath)
{
    if (!EvtClearLog(NULL, pwsPath, NULL, 0))
    {
        wprintf(L"EvtClearLog failed with %lu\n", GetLastError());
    }
}

void main()
{
    EVT_HANDLE hChannelEnum = NULL;
    DWORD status = ERROR_SUCCESS;
    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    LPWSTR pChannelPath = NULL;

    if (!IsRunAsAdmin())
    {
        AttemptRunAsAdmin();
        return;
    }

    // Enumerate the channels
    hChannelEnum = EvtOpenChannelEnum(NULL, 0);
    if (NULL == hChannelEnum)
    {
        wprintf(L"EvtOpenChannelEnum failed with %lu\n", GetLastError());
        goto cleanup;
    }

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
                    wprintf(L"malloc failed\n");
                    goto cleanup;
                }
            }

            if (ERROR_SUCCESS != (status = GetLastError()))
            {
                wprintf(L"EvtNextChannelPath failed with %lu\n", status);
                goto cleanup;
            }
        }

        wprintf(L"%s\n", pChannelPath);

        // Clear the event log
        ClearEventLogCustom(pChannelPath);
    }

cleanup:

    if (hChannelEnum)
        EvtClose(hChannelEnum);

    if (pChannelPath)
        free(pChannelPath);

system("pause");
}
