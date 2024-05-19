#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef PSAPI_VERSION
#define PSAPI_VERSION 1
#endif

#include <winstrct.h>
#include <ntdll.h>
#include <Psapi.h>

#define THREAD_STACK_SIZE (1 << 20)

#ifdef _WIN64
#pragma comment(lib, "psapi.lib")
#endif

HMODULE
WINAPI
LoadRemoteLibrary(HANDLE process, LPCWSTR dllpath)
{
    SIZE_T towrite = (wcslen(dllpath) + 1) * sizeof(*dllpath);

    LPVOID remote_data_ptr = NULL;

    NTSTATUS status = NtAllocateVirtualMemory(process, &remote_data_ptr, 0, &towrite, MEM_COMMIT, PAGE_READWRITE);

    if (!NT_SUCCESS(status))
    {
        SetLastError(RtlNtStatusToDosError(status));
        return NULL;
    }

    SIZE_T written;

    if (!WriteProcessMemory(process, remote_data_ptr, dllpath, towrite, &written))
    {
        SIZE_T region_size = 0;
        NtFreeVirtualMemory(process, &remote_data_ptr, &region_size, MEM_RELEASE);

        return NULL;
    }

    DWORD tid;
    HANDLE thread = CreateRemoteThread(process, NULL, THREAD_STACK_SIZE, (LPTHREAD_START_ROUTINE)LoadLibraryW, remote_data_ptr, 0, &tid);

    if (thread == NULL)
    {
        SIZE_T region_size = 0;
        NtFreeVirtualMemory(process, &remote_data_ptr, &region_size, MEM_RELEASE);

        return NULL;
    }

    WaitForSingleObject(thread, INFINITE);

    SIZE_T region_size = 0;
    NtFreeVirtualMemory(process, &remote_data_ptr, &region_size, MEM_RELEASE);

    HMODULE module = NULL;

#ifndef _WIN64

    if (!GetExitCodeThread(thread, (LPDWORD) & module))
    {
        CloseHandle(thread);
        return NULL;
    }

    CloseHandle(thread);

#else

    CloseHandle(thread);

    DWORD length = 4096;
    WHeapMem<HMODULE> modules;
    
    for (;;)
    {
        modules.ReAlloc(length, HEAP_GENERATE_EXCEPTIONS);

        BOOL rc = EnumProcessModules(process, modules, (DWORD)modules.GetSize(), &length);

        if (length > (DWORD)modules.GetSize())
        {
            continue;
        }

        if (!rc)
        {
            return NULL;
        }

        break;
    }

    int items = length / sizeof(HMODULE);

    LPCWSTR dll_base_path = wcsrchr(dllpath, '\\');

    if (dll_base_path == NULL)
    {
        dll_base_path = dllpath;
    }
    else
    {
        dll_base_path++;
    }

    WCHAR modname[MAX_PATH];

    for (int i = 0; i < items; i++)
    {
        DWORD rc = GetModuleBaseName(process, modules[i], modname, _countof(modname));
        modname[rc] = 0;

        if (_wcsicmp(modname, dll_base_path) == 0)
        {
            module = modules[i];
            break;
        }
    }

#endif

    if (module == NULL)
    {
        SetLastError(ERROR_NOT_FOUND);
    }

    return module;
}

