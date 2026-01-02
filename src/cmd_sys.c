#include "tnr.h"
#include <tlhelp32.h>

// --- TCC / Manual Compat for Process ---
#ifdef __TINYC__
#ifndef TH32CS_SNAPPROCESS
#define TH32CS_SNAPPROCESS 0x00000002
#endif
#endif

// Helper for memory
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
    char user[256];
    DWORD len = 256;
    if (GetUserName(user, &len)) {
        printf("User: %s\n", user);
    } else {
        printf("Error getting username.\n");
    }
}

void cmd_hostname(char** args, int c) {
    char host[256];
    DWORD len = 256;
    if (GetComputerName(host, &len)) {
        printf("Hostname: %s\n", host);
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
    LPTCH lpvEnv = GetEnvironmentStrings(); 
    if (lpvEnv == NULL) return;
    LPTSTR lpszVariable = (LPTSTR) lpvEnv; 
    while (*lpszVariable) { 
        printf("%s\n", lpszVariable); 
        lpszVariable += lstrlen(lpszVariable) + 1; 
    } 
    FreeEnvironmentStrings(lpvEnv);
}

void cmd_proc(char** args, int c) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(pe);
        if (Process32First(hSnap, &pe)) {
            print_header("PROCESS LIST");
            printf("%-8s %s\n", "PID", "NAME");
            printf("%-8s %s\n", "---", "----");
            int limit = 0;
            do {
                printf("%-8lu %s\n", pe.th32ProcessID, pe.szExeFile);
                if (++limit > 40) { printf("... (Limit reached)\n"); break; }
            } while(Process32Next(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }
}

// [NEW] Enhanced Process List
void cmd_ps(char** args, int c) {
    cmd_proc(args, c); // Wrapper for now, can be enhanced later with memory usage if needed
}

// [NEW] Fetch ASCII Art
void cmd_fetch(char** args, int c) {
    char computer[256];
    char user[256];
    DWORD len = 256;
    GetComputerName(computer, &len);
    len = 256;
    GetUserName(user, &len);

    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);

    set_col(C_HACK); // Special Green
    printf("\n");
    printf("   .-------.    %s@%s\n", user, computer);
    printf("   |  |>_  |    ----------------------\n");
    printf("   |       |    OS: Windows Native\n");
    printf("   |       |    Sh: TNRM1N4L v%s\n", VERSION);
    printf("   |_______|    RAM: %llu / %llu MB\n", (mem.ullTotalPhys - mem.ullAvailPhys)/1024/1024, mem.ullTotalPhys/1024/1024);
    printf("                Uptime: %ld min\n", GetTickCount()/60000);
    printf("\n");
    set_col(C_RESET);
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
