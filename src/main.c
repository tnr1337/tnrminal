#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

// --- Missing Definitions for TCC ---
#ifndef TH32CS_SNAPPROCESS
#define TH32CS_SNAPPROCESS 0x00000002
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

// --- System Definitions ---
#define MAX_CMD_LEN 256
#define MAX_ARGS 16

// Colors
#define COL_RESET   7
#define COL_GREEN   10
#define COL_RED     12
#define COL_CYAN    11
#define COL_YELLOW  14

// --- Global State ---
HANDLE hConsole;
int running = 1;

// --- Helper Functions ---
void set_color(int color) {
    SetConsoleTextAttribute(hConsole, color);
}

void print_header() {
    set_color(COL_GREEN);
    printf("TNRM1N4L v1.0 [KERNEL MODE]\n");
    set_color(COL_RESET);
}

void print_time() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    printf("[%02d:%02d:%02d.%03d] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

// --- Commands ---

// 1. SYS: System Monitor
typedef BOOL (WINAPI *PtrGlobalMemoryStatusEx)(LPMEMORYSTATUSEX);

void cmd_sys() {
    HMODULE hKernel = GetModuleHandle("kernel32.dll");
    if (!hKernel) return;

    PtrGlobalMemoryStatusEx pGlobalMemoryStatusEx = (PtrGlobalMemoryStatusEx)GetProcAddress(hKernel, "GlobalMemoryStatusEx");
    if (!pGlobalMemoryStatusEx) {
        printf("Error: GlobalMemoryStatusEx not supported.\n");
        return;
    }

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    pGlobalMemoryStatusEx(&memInfo);

    DWORDLONG totalPhys = memInfo.ullTotalPhys / 1024 / 1024;
    DWORDLONG usedPhys = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / 1024 / 1024;
    
    set_color(COL_CYAN);
    printf("\n--- SYSTEM MONITOR ---\n");
    printf("RAM Total : %llu MB\n", totalPhys);
    printf("RAM Used  : %llu MB\n", usedPhys);
    printf("Memory Load: %ld%%\n", memInfo.dwMemoryLoad);
    set_color(COL_RESET);
}

// 3. PROC: Process Viewer (Dynamic Loading)
void cmd_proc() {
    HMODULE hKernel = GetModuleHandle("kernel32.dll");
    if (!hKernel) return;

    PtrCreateToolhelp32Snapshot pCreateSnapshot = (PtrCreateToolhelp32Snapshot)GetProcAddress(hKernel, "CreateToolhelp32Snapshot");
    PtrProcess32First pProcessFirst = (PtrProcess32First)GetProcAddress(hKernel, "Process32First");
    PtrProcess32Next pProcessNext = (PtrProcess32Next)GetProcAddress(hKernel, "Process32Next");

    if (!pCreateSnapshot || !pProcessFirst || !pProcessNext) {
        printf("Error: Could not load process APIs.\n");
        return;
    }

    HANDLE hSnapshot = pCreateSnapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (pProcessFirst(hSnapshot, &pe32)) {
        set_color(COL_YELLOW);
        printf("\nPID\t\tNAME\n");
        printf("------\t\t--------------------\n");
        set_color(COL_RESET);
        int count = 0;
        do {
            printf("%d\t\t%s\n", pe32.th32ProcessID, pe32.szExeFile);
            count++;
            if (count > 20) {
                printf("... (Limit reached, use 'all' for more)\n");
                break;
            }
        } while (pProcessNext(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
}

// 4. HEX: Hex Dump
void cmd_hex(const char* filename) {
    if (!filename) {
        printf("Usage: hex <filename>\n");
        return;
    }

    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("Error: Could not open file '%s'\n", filename);
        return;
    }

    unsigned char buffer[16];
    size_t bytesRead;
    size_t offset = 0;

    printf("\n");
    while ((bytesRead = fread(buffer, 1, 16, fp)) > 0) {
        set_color(COL_CYAN);
        printf("%08zX  ", offset); // Offset
        set_color(COL_RESET);

        // Hex bytes
        for (size_t i = 0; i < 16; i++) {
            if (i < bytesRead) printf("%02X ", buffer[i]);
            else printf("   ");
        }

        printf(" |");
        // ASCII char
        for (size_t i = 0; i < bytesRead; i++) {
            if (buffer[i] >= 32 && buffer[i] <= 126) printf("%c", buffer[i]);
            else printf(".");
        }
        printf("|\n");
        offset += 16;
        if (offset > 512) { // Safety break
             printf("... (Truncated for view)\n");
             break;
        }
    }
    fclose(fp);
}

// 5. LS: List Directory
void cmd_ls() {
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile("*", &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Error listing directory.\n");
        return;
    }

    set_color(COL_CYAN);
    printf("\n  Attrib\tSize\t\tName\n");
    printf("  ------\t----\t\t----\n");
    set_color(COL_RESET);

    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            set_color(COL_YELLOW);
            printf("  <DIR>\t\t-\t\t%s\n", findData.cFileName);
        } else {
            set_color(COL_RESET);
            printf("  FILE\t\t%ld\t\t%s\n", (findData.nFileSizeHigh * (MAXDWORD+1)) + findData.nFileSizeLow, findData.cFileName);
        }
    } while (FindNextFile(hFind, &findData));
    FindClose(hFind);
    set_color(COL_RESET);
}

// 7. MKFILE: Create File
void cmd_mkfile(const char* filename) {
    if (!filename) {
        printf("Usage: mkfile <filename>\n");
        return;
    }
    FILE* fp = fopen(filename, "w");
    if (fp) {
        fprintf(fp, "Created by TNRM1N4L");
        fclose(fp);
        printf("File '%s' created.\n", filename);
    } else {
        printf("Error creating file.\n");
    }
}

// 9. MATRIX: Screensaver
void cmd_matrix() {
    system("cls");
    set_color(COL_GREEN);
    printf("Entering MATRIX mode. Press CTRL+C to Abort (TODO: Async implementation)\n");
    Sleep(1000);
    // Note: A true async matrix needs non-blocking input, simplifying for day 1
    for(int i=0; i<500; i++) {
        for(int j=0; j<80; j++) {
            if (rand() % 10 > 8) printf("%c", (rand() % 94) + 33);
            else printf(" ");
        }
        // Sleep(10); // Too fast is cool
    }
    set_color(COL_RESET);
    system("cls");
    printf("Matrix disconnected.\n");
}

void cmd_help() {
    printf("\n--- AVAILABLE COMMANDS (v1.0) ---\n");
    printf(" sys      : System Monitor (RAM/CPU)\n");
    printf(" proc     : Process Viewer\n");
    printf(" ls       : List Directory\n");
    printf(" hex      : Hex Dump Viewer <file>\n");
    printf(" mkfile   : Create File <name>\n");
    printf(" matrix   : Screensaver\n");
    printf(" time     : Show Nano Clock\n");
    printf(" cls      : Clear Screen\n");
    printf(" shutdown : Exit TNRM1N4L\n");
}

int main() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // Init Visuals
    system("cls");
    print_header();
    print_time();
    printf("\n");

    char input[MAX_CMD_LEN];
    char* args[MAX_ARGS];

    while (running) {
        set_color(COL_GREEN);
        printf("\nTNRM1N4L> ");
        set_color(COL_RESET);

        if (!fgets(input, MAX_CMD_LEN, stdin)) break;
        input[strcspn(input, "\n")] = 0; // Remove newline

        // Parse logic
        int arg_count = 0;
        char* token = strtok(input, " ");
        while (token != NULL && arg_count < MAX_ARGS) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }

        if (arg_count == 0) continue;

        if (strcmp(args[0], "shutdown") == 0 || strcmp(args[0], "exit") == 0) {
            running = 0;
            printf("Shutting down kernel...\n");
        }
        else if (strcmp(args[0], "sys") == 0) cmd_sys();
        else if (strcmp(args[0], "proc") == 0) cmd_proc();
        else if (strcmp(args[0], "ls") == 0) cmd_ls();
        else if (strcmp(args[0], "hex") == 0) cmd_hex(arg_count > 1 ? args[1] : NULL);
        else if (strcmp(args[0], "mkfile") == 0) cmd_mkfile(arg_count > 1 ? args[1] : NULL);
        else if (strcmp(args[0], "matrix") == 0) cmd_matrix(); 
        else if (strcmp(args[0], "time") == 0) { print_time(); printf("\n"); }
        else if (strcmp(args[0], "cls") == 0) system("cls"); 
        else if (strcmp(args[0], "help") == 0) cmd_help();
        else {
            printf("Unknown command: %s\n", args[0]);
        }
    }

    return 0;
}
