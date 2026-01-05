#include "tnr.h"
// #include <tlhelp32.h> // Removed for TCC compatibility

// --- TCC Compatibility & Manual Definitions ---
#ifndef TH32CS_SNAPPROCESS
#define TH32CS_SNAPPROCESS 0x00000002
#endif

// TCC Helper
#ifdef __TINYC__
LPCH WINAPI GetEnvironmentStrings(void);
BOOL WINAPI FreeEnvironmentStrings(LPCH);
#endif

typedef struct tagPROCESSENTRY32 {
    DWORD     dwSize;
    DWORD     cntUsage;
    DWORD     th32ProcessID;
    ULONG_PTR th32DefaultHeapID;
    DWORD     th32ModuleID;
    DWORD     cntThreads;
    DWORD     th32ParentProcessID;
    LONG      pcPriClassBase;
    DWORD     dwFlags;
    CHAR      szExeFile[MAX_PATH];
} PROCESSENTRY32;

typedef HANDLE (WINAPI *PtrCreateToolhelp32Snapshot)(DWORD, DWORD);
typedef BOOL (WINAPI *PtrProcess32First)(HANDLE, PROCESSENTRY32*);
typedef BOOL (WINAPI *PtrProcess32Next)(HANDLE, PROCESSENTRY32*);
typedef BOOL (WINAPI *PtrGlobalMemoryStatusEx)(LPMEMORYSTATUSEX);

void cmd_sys(char** args, int c) {
    HMODULE hKernel = GetModuleHandle("kernel32.dll");
    PtrGlobalMemoryStatusEx pGlobalMemoryStatusEx = (PtrGlobalMemoryStatusEx)GetProcAddress(hKernel, "GlobalMemoryStatusEx");
    
    if (pGlobalMemoryStatusEx) {
        MEMORYSTATUSEX mem;
        mem.dwLength = sizeof(mem);
        pGlobalMemoryStatusEx(&mem);
        print_header("SYSTEM MONITOR");
        printf("RAM Total: %llu MB\n", mem.ullTotalPhys/1024/1024);
        printf("RAM Free : %llu MB\n", mem.ullAvailPhys/1024/1024);
        printf("Page File: %llu MB\n", mem.ullTotalPageFile/1024/1024);
        printf("Load     : %ld%%\n", mem.dwMemoryLoad);
    }
}

void cmd_whoami(char** args, int c) {
    char user[MAX_USERNAME];
    DWORD len = MAX_USERNAME;
    if (GetUserName(user, &len)) {
        set_col(C_OK);
        printf("User: %s\n", user);
        set_col(C_RESET);
    } else {
        print_error("Could not retrieve username.");
    }
}

void cmd_hostname(char** args, int c) {
    char host[MAX_HOSTNAME];
    DWORD len = MAX_HOSTNAME;
    if (GetComputerName(host, &len)) {
        set_col(C_OK);
        printf("Hostname: %s\n", host);
        set_col(C_RESET);
    } else {
        print_error("Could not retrieve hostname.");
    }
}

void cmd_os(char** args, int c) {
    // Basic OS info by calling system
    system("ver");
}

void cmd_uptime(char** args, int c) {
    DWORD ticks = GetTickCount();
    unsigned int sec = ticks / 1000;
    unsigned int min = sec / 60;
    unsigned int hour = min / 60;
    printf("System Uptime: %u hours, %u minutes, %u seconds\n", hour, min%60, sec%60);
}

void cmd_drives(char** args, int c) {
    DWORD drives = GetLogicalDrives();
    print_header("LOGICAL DRIVES");
    for(int i=0; i<26; i++) {
        if ((drives >> i) & 1) {
            printf("%c:\\ ", 'A'+i);
        }
    }
    printf("\n");
}

void cmd_env(char** args, int c) {
#ifdef __TINYC__
    LPTSTR lpvEnv = (LPTSTR)GetEnvironmentStrings();
#else
    LPTCH lpvEnv = GetEnvironmentStrings(); 
#endif
    if (lpvEnv == NULL) return;
    LPTSTR lpszVariable = (LPTSTR) lpvEnv; 
    while (*lpszVariable) { 
        printf("%s\n", lpszVariable); 
        lpszVariable += lstrlen(lpszVariable) + 1; 
    } 
    FreeEnvironmentStrings(lpvEnv);
}

void cmd_proc(char** args, int c) {
    HMODULE hKernel = GetModuleHandle("kernel32.dll");
    PtrCreateToolhelp32Snapshot pSnap = (PtrCreateToolhelp32Snapshot)GetProcAddress(hKernel, "CreateToolhelp32Snapshot");
    PtrProcess32First pFirst = (PtrProcess32First)GetProcAddress(hKernel, "Process32First");
    PtrProcess32Next pNext = (PtrProcess32Next)GetProcAddress(hKernel, "Process32Next");

    if (pSnap && pFirst && pNext) {
        HANDLE hSnap = pSnap(TH32CS_SNAPPROCESS, 0);
        if (hSnap != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe;
            pe.dwSize = sizeof(pe);
            if (pFirst(hSnap, &pe)) {
                print_header("PROCESS LIST");
                set_col(C_INFO);
                printf("%-8s %-6s %s\n", "PID", "THR", "NAME");
                printf("%-8s %-6s %s\n", "---", "---", "----");
                set_col(C_RESET);
                int count = 0;
                do {
                    printf("%-8lu %-6lu %s\n", pe.th32ProcessID, pe.cntThreads, pe.szExeFile);
                    if (++count > PROC_DISPLAY_LIMIT) {
                        set_col(C_WARN);
                        printf("... (%d limit reached, use 'ps -a' for more)\n", PROC_DISPLAY_LIMIT);
                        set_col(C_RESET);
                        break;
                    }
                } while(pNext(hSnap, &pe));
                set_col(C_INFO);
                printf("\nTotal: %d processes shown\n", count);
                set_col(C_RESET);
            }
            CloseHandle(hSnap);
        }
    } else {
        print_error("Process listing not supported on this kernel.");
    }
}

// [NEW] Enhanced Process List
void cmd_ps(char** args, int c) {
    // Check for -a flag for all processes
    int show_all = 0;
    if (c >= 2 && strcmp(args[1], "-a") == 0) {
        show_all = 1;
    }
    
    HMODULE hKernel = GetModuleHandle("kernel32.dll");
    PtrCreateToolhelp32Snapshot pSnap = (PtrCreateToolhelp32Snapshot)GetProcAddress(hKernel, "CreateToolhelp32Snapshot");
    PtrProcess32First pFirst = (PtrProcess32First)GetProcAddress(hKernel, "Process32First");
    PtrProcess32Next pNext = (PtrProcess32Next)GetProcAddress(hKernel, "Process32Next");

    if (pSnap && pFirst && pNext) {
        HANDLE hSnap = pSnap(TH32CS_SNAPPROCESS, 0);
        if (hSnap != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe;
            pe.dwSize = sizeof(pe);
            if (pFirst(hSnap, &pe)) {
                print_header("PROCESSES");
                set_col(C_INFO);
                printf("%-8s %-8s %-6s %s\n", "PID", "PPID", "THR", "NAME");
                printf("%-8s %-8s %-6s %s\n", "---", "----", "---", "----");
                set_col(C_RESET);
                int count = 0;
                do {
                    printf("%-8lu %-8lu %-6lu %s\n", 
                           pe.th32ProcessID, 
                           pe.th32ParentProcessID,
                           pe.cntThreads, 
                           pe.szExeFile);
                    count++;
                    if (!show_all && count > PROC_DISPLAY_LIMIT) {
                        set_col(C_WARN);
                        printf("... (use 'ps -a' for all)\n");
                        set_col(C_RESET);
                        break;
                    }
                } while(pNext(hSnap, &pe));
                set_col(C_INFO);
                printf("\nTotal: %d processes\n", count);
                set_col(C_RESET);
            }
            CloseHandle(hSnap);
        }
    } else {
        print_error("Process listing not supported.");
    }
}

// [NEW] Fetch ASCII Art - Enhanced
void cmd_fetch(char** args, int c) {
    char computer[256];
    char user[256];
    DWORD len = 256;
    GetComputerName(computer, &len);
    len = 256;
    GetUserName(user, &len);

    HMODULE hKernel = GetModuleHandle("kernel32.dll");
    PtrGlobalMemoryStatusEx pGlobalMemoryStatusEx = (PtrGlobalMemoryStatusEx)GetProcAddress(hKernel, "GlobalMemoryStatusEx");
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    if(pGlobalMemoryStatusEx) pGlobalMemoryStatusEx(&mem);
    
    DWORD ticks = GetTickCount();
    int hours = ticks / 3600000;
    int mins = (ticks % 3600000) / 60000;

    SYSTEM_INFO si;
    GetSystemInfo(&si);

    printf("\n");
    set_col(C_HACK);
    printf("        _____  _   _  ____   __  __  __   _   _    _    _     \n");
    printf("       |_   _|| \\ | ||  _ \\ |  \\/  ||  | | \\ | |  / \\  | |    ");
    set_col(C_OK); printf("%s", user); set_col(C_RESET); printf("@"); set_col(C_INFO); printf("%s\n", computer);
    set_col(C_HACK);
    printf("         | |  |  \\| || |_) || |\\/| || | |  \\| | / _ \\ | |    ");
    set_col(C_RESET); printf("-----------------------------\n");
    set_col(C_HACK);
    printf("         | |  | |\\  ||  _ < | |  | || | | |\\  |/ ___ \\| |___ ");
    set_col(C_INFO); printf("OS: "); set_col(C_RESET); printf("Windows Native\n");
    set_col(C_HACK);
    printf("         |_|  |_| \\_||_| \\_\\|_|  |_||_| |_| \\_/_/   \\_\\_____|");
    set_col(C_INFO); printf("Shell: "); set_col(C_RESET); printf("TNRM1N4L v%s\n", VERSION);
    printf("                                                        ");
    set_col(C_INFO); printf("CPU: "); set_col(C_RESET); printf("%lu cores\n", si.dwNumberOfProcessors);
    printf("                                                        ");
    if(pGlobalMemoryStatusEx) {
        unsigned long long used = (mem.ullTotalPhys - mem.ullAvailPhys)/1024/1024;
        unsigned long long total = mem.ullTotalPhys/1024/1024;
        set_col(C_INFO); printf("Memory: "); set_col(C_RESET); 
        printf("%llu / %llu MB (%lu%%)\n", used, total, mem.dwMemoryLoad);
    }
    printf("                                                        ");
    set_col(C_INFO); printf("Uptime: "); set_col(C_RESET); printf("%d hours, %d mins\n", hours, mins);
    printf("\n");
    printf("                                                        ");
    // Color palette
    for(int i = 0; i < 8; i++) { set_col(i); printf("███"); }
    printf("\n                                                        ");
    for(int i = 8; i < 16; i++) { set_col(i); printf("███"); }
    set_col(C_RESET);
    printf("\n\n");
}

void cmd_time(char** args, int c) {
    SYSTEMTIME st; GetLocalTime(&st);
    printf("%02d:%02d:%02d.%03d\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

void cmd_date(char** args, int c) {
    SYSTEMTIME st; GetLocalTime(&st);
    printf("%02d/%02d/%04d\n", st.wDay, st.wMonth, st.wYear);
}

// [NEW] Simulations
void cmd_shutdown(char** args, int c) {
    printf("Simulating shutdown...\n");
    tnr_sleep(1000);
    running = 0;
}

void cmd_reboot(char** args, int c) {
    printf("Rebooting...\n");
    tnr_sleep(1000);
    system("cls");
    printf("TNRM1N4L Reloaded.\n");
}

void cmd_lock(char** args, int c) {
    system("cls");
    printf("SESSION LOCKED. Enter password to unlock.\n");
    char pass[64];
    while(1) {
        printf("Password: ");
        if (fgets(pass, 64, stdin)) {
            // Any password works for now, it's a mock
            if (strlen(pass) > 1) {
                printf("Unlocked.\n");
                break;
            }
        }
    }
}
