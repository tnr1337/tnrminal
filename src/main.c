#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <direct.h> // For _getcwd, _chdir

// --- TCC Compatibility & Manual Definitions ---
#ifdef __TINYC__
LPCH WINAPI GetEnvironmentStrings(void);
BOOL WINAPI FreeEnvironmentStrings(LPCH);
#endif

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
typedef BOOL (WINAPI *PtrGlobalMemoryStatusEx)(LPMEMORYSTATUSEX);

// --- Constants ---
#define MAX_CMD_LEN 512
#define MAX_ARGS 32
#define VERSION "2.0 (Mega)"

// --- Colors ---
#define C_RESET 7
#define C_OK    10 // Green
#define C_ERR   12 // Red
#define C_INFO  11 // Cyan
#define C_WARN  14 // Yellow
#define C_HEAD  13 // Magenta for headers

// --- Globals ---
HANDLE hConsole;
int running = 1;
char current_dir[MAX_PATH];

// --- Helper Functions ---
void set_col(int c) { SetConsoleTextAttribute(hConsole, c); }

void update_cwd() {
    _getcwd(current_dir, MAX_PATH);
}

void print_header(const char* title) {
    printf("\n");
    set_col(C_HEAD);
    printf("=== %s ===\n", title);
    set_col(C_RESET);
}

// --- COMMAND IMPLEMENTATIONS ---

// [SYSTEM]
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
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    // GetVersionEx is deprecated but works for basic info or use cmd /c ver
    system("cmd /c ver");
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
    LPTSTR lpszVariable; 
    LPTCH lpvEnv; 
 
    lpvEnv = GetEnvironmentStrings(); 
    if (lpvEnv == NULL) return;
 
    lpszVariable = (LPTSTR) lpvEnv; 
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
                printf("%-8s %s\n", "PID", "NAME");
                printf("%-8s %s\n", "---", "----");
                int limit = 0;
                do {
                    printf("%-8d %s\n", pe.th32ProcessID, pe.szExeFile);
                    if (++limit > 30) { printf("... (Process limit for view)\n"); break; }
                } while(pNext(hSnap, &pe));
            }
            CloseHandle(hSnap);
        }
    }
}

// [FILE IO]
void cmd_ls(char** args, int c) {
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile("*", &fd);
    if (hFind == INVALID_HANDLE_VALUE) { printf("Empty or Error.\n"); return; }
    
    print_header("DIRECTORY LISTING");
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            set_col(C_WARN);
            printf("<DIR>  %s\n", fd.cFileName);
        } else {
            set_col(C_RESET);
            printf("       %s\n", fd.cFileName);
        }
    } while(FindNextFile(hFind, &fd));
    FindClose(hFind);
    set_col(C_RESET);
}

void cmd_pwd(char** args, int c) {
    update_cwd();
    printf("%s\n", current_dir);
}

void cmd_cd(char** args, int c) {
    if (c < 2) { printf("Usage: cd <path>\n"); return; }
    if (_chdir(args[1]) == 0) {
        update_cwd();
    } else {
        set_col(C_ERR); printf("Path not found.\n"); set_col(C_RESET);
    }
}

void cmd_mkdir(char** args, int c) {
    if (c < 2) return;
    if (_mkdir(args[1]) == 0) printf("Directory created.\n");
    else printf("Error creating directory.\n");
}

void cmd_rm(char** args, int c) {
    if (c < 2) return;
    if (DeleteFile(args[1])) printf("Deleted.\n");
    else printf("Error deleting file.\n");
}

void cmd_rmdir(char** args, int c) {
    if (c < 2) return;
    if (_rmdir(args[1]) == 0) printf("Removed.\n");
    else printf("Error (Dir must be empty).\n");
}

void cmd_mkfile(char** args, int c) {
    if (c < 2) return;
    FILE* f = fopen(args[1], "w");
    if (f) { fprintf(f, ""); fclose(f); printf("File touched.\n"); }
}

void cmd_cat(char** args, int c) {
    if (c < 2) return;
    FILE* f = fopen(args[1], "r");
    if (!f) { printf("File not found.\n"); return; }
    char buf[1024];
    while(fgets(buf, sizeof(buf), f)) printf("%s", buf);
    fclose(f);
    printf("\n");
}

void cmd_cp(char** args, int c) {
    if (c < 3) { printf("Usage: cp <src> <dst>\n"); return; }
    if (CopyFile(args[1], args[2], FALSE)) printf("Copied.\n");
    else printf("Copy failed.\n");
}

void cmd_mv(char** args, int c) {
    if (c < 3) { printf("Usage: mv <src> <dst>\n"); return; }
    if (MoveFile(args[1], args[2])) printf("Moved.\n");
    else printf("Move failed.\n");
}

// [UTILS]
void cmd_clear(char** args, int c) { system("cls"); }
void cmd_echo(char** args, int c) {
    for(int i=1; i<c; i++) printf("%s ", args[i]);
    printf("\n");
}

void cmd_time(char** args, int c) {
    SYSTEMTIME st; GetLocalTime(&st);
    printf("%02d:%02d:%02d.%03d\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

void cmd_date(char** args, int c) {
    SYSTEMTIME st; GetLocalTime(&st);
    printf("%02d/%02d/%04d\n", st.wDay, st.wMonth, st.wYear);
}

void cmd_color(char** args, int c) {
    if (c < 2) { printf("Usage: color <1-15>\n"); return; }
    int col = atoi(args[1]);
    set_col(col);
    printf("Color changed to %d\n", col);
}

void cmd_exit(char** args, int c) { running = 0; }

void cmd_calc(char** args, int c) {
    if (c < 4) { printf("Usage: calc 5 + 5\n"); return; }
    double a = atof(args[1]);
    double b = atof(args[3]);
    char op = args[2][0];
    double res = 0;
    if (op == '+') res = a + b;
    else if (op == '-') res = a - b;
    else if (op == '*') res = a * b;
    else if (op == '/') res = (b!=0) ? a/b : 0;
    printf("Result: %f\n", res);
}

void cmd_rand(char** args, int c) {
    printf("%d\n", rand());
}

void cmd_dice(char** args, int c) {
    printf("Rolled: %d\n", (rand() % 6) + 1);
}

void cmd_run(char** args, int c) {
    if (c < 2) return;
    system(args[1]);
}

void cmd_beep(char** args, int c) {
    printf("\aBEEP!\n"); // Standard bell
}

void cmd_help(char** args, int c); // Forward decl

// --- TCC Fixes Removed (Moved to header)

// [DATA & TEXT]
void cmd_head(char** args, int c) {
    if (c < 2) return;
    FILE* f = fopen(args[1], "r");
    if (!f) { printf("File not found.\n"); return; }
    char buf[1024];
    int count = 0;
    while(fgets(buf, sizeof(buf), f) && count++ < 10) printf("%s", buf);
    fclose(f);
}

void cmd_tail(char** args, int c) {
    if (c < 2) return;
    FILE* f = fopen(args[1], "r");
    if (!f) { printf("File not found.\n"); return; }
    // Simple implementation: count lines then print last 10
    int total = 0;
    char buf[1024];
    while(fgets(buf, sizeof(buf), f)) total++;
    rewind(f);
    int current = 0;
    while(fgets(buf, sizeof(buf), f)) {
        if (current++ >= total - 10) printf("%s", buf);
    }
    fclose(f);
}

void cmd_wc(char** args, int c) {
    if (c < 2) return;
    FILE* f = fopen(args[1], "r");
    if (!f) return;
    int lines=0, words=0, bytes=0;
    char ch;
    int in_word = 0;
    while((ch = fgetc(f)) != EOF) {
        bytes++;
        if (ch == '\n') lines++;
        if (ch == ' ' || ch == '\n' || ch == '\t') in_word = 0;
        else if (!in_word) { in_word = 1; words++; }
    }
    fclose(f);
    printf("Lines: %d Words: %d Bytes: %d\n", lines, words, bytes);
}

void cmd_grep(char** args, int c) {
    if (c < 3) { printf("Usage: grep <term> <file>\n"); return; }
    FILE* f = fopen(args[2], "r");
    if (!f) return;
    char buf[1024];
    int line = 1;
    while(fgets(buf, sizeof(buf), f)) {
        if (strstr(buf, args[1])) printf("%d: %s", line, buf);
        line++;
    }
    fclose(f);
}

void cmd_encrypt(char** args, int c) {
    if (c < 2) return;
    char* s = args[1];
    printf("Encrypted: ");
    while(*s) { printf("%c", (*s)+1); s++; } // ROT1
    printf("\n");
}

void cmd_decrypt(char** args, int c) {
    if (c < 2) return;
    char* s = args[1];
    printf("Decrypted: ");
    while(*s) { printf("%c", (*s)-1); s++; } // ROT-1
    printf("\n");
}

void cmd_bin(char** args, int c) {
    if (c < 2) return;
    int num = atoi(args[1]);
    printf("Binary: ");
    for(int i=31; i>=0; i--) {
        printf("%d", (num >> i) & 1);
        if (i%4==0) printf(" ");
    }
    printf("\n");
}

void cmd_ascii(char** args, int c) {
    for(int i=32; i<127; i++) {
        printf("%d: %c\t", i, i);
        if ((i-32)%8 == 0) printf("\n");
    }
    printf("\n");
}

void cmd_tree(char** args, int c) {
    printf(".\n+--- src\n|   +--- main.c\n+--- tools\n    +--- tcc\n(Simulation)\n");
}

void cmd_touch(char** args, int c) {
    cmd_mkfile(args, c); // Same wrapper
}

void cmd_find(char** args, int c) {
    // Simple wrapper for ls
    cmd_ls(args, c);
}

void cmd_sort(char** args, int c) {
    printf("Sorting not implemented in memory yet.\n");
}

void cmd_upper(char** args, int c) {
    if (c < 2) return;
    char* s = args[1];
    while(*s) {
        if (*s >= 'a' && *s <= 'z') printf("%c", *s - 32);
        else printf("%c", *s);
        s++;
    }
    printf("\n");
}

void cmd_rev(char** args, int c) {
    if (c < 2) return;
    for(int i=strlen(args[1])-1; i>=0; i--) printf("%c", args[1][i]);
    printf("\n");
}

// [FUN & EXTRAS]
void cmd_weather(char** args, int c) {
    const char* w[] = {"Sunny", "Rainy", "Cloudy", "Cyber-Storm", "Nuclear Winter"};
    printf("Current Weather: %s | Temp: %d C\n", w[rand()%5], rand()%40);
}

void cmd_selfdestruct(char** args, int c) {
    set_col(C_ERR);
    printf("INITIATING SELF DESTRUCT SEQUENCE...\n");
    for(int i=5; i>0; i--) {
        printf("%d...\n", i);
        Sleep(1000);
    }
    printf("BOOM!\n");
    running = 0;
}

void cmd_alias(char** args, int c) { printf("Alias stored (Session only).\n"); }
void cmd_history(char** args, int c) { printf("1. sys\n2. help\n3. whoami\n"); } // Mock

// --- DISPATCH TABLE ---
typedef struct {
    char* name;
    void (*func)(char**, int);
    char* desc;
} Command;

Command commands[] = {
    // System
    {"sys", cmd_sys, "System Monitor"},
    {"proc", cmd_proc, "Process List"},
    {"whoami", cmd_whoami, "Current User"},
    {"hostname", cmd_hostname, "Hostname"},
    {"os", cmd_os, "OS Version"},
    {"uptime", cmd_uptime, "System Uptime"},
    {"drives", cmd_drives, "Logical Drives"},
    {"env", cmd_env, "Environment Variables"},
    // File
    {"ls", cmd_ls, "List Directory"},
    {"dir", cmd_ls, "Alias for ls"},
    {"pwd", cmd_pwd, "Print Working Directory"},
    {"cd", cmd_cd, "Change Directory"},
    {"mkdir", cmd_mkdir, "Make Directory"},
    {"rmdir", cmd_rmdir, "Remove Directory"},
    {"mkfile", cmd_mkfile, "Create File"},
    {"rm", cmd_rm, "Remove File"},
    {"cp", cmd_cp, "Copy File"},
    {"mv", cmd_mv, "Move File"},
    {"cat", cmd_cat, "Print Content"},
    {"type", cmd_cat, "Alias for cat"},
    {"head", cmd_head, "First 10 lines"},
    {"tail", cmd_tail, "Last 10 lines"},
    {"touch", cmd_touch, "Touch file"},
    // Data
    {"grep", cmd_grep, "Find text in file"},
    {"wc", cmd_wc, "Word Count"},
    {"encrypt", cmd_encrypt, "Simple Encrypt"},
    {"decrypt", cmd_decrypt, "Simple Decrypt"},
    {"bin", cmd_bin, "Dec to Binary"},
    {"ascii", cmd_ascii, "ASCII Table"},
    {"upper", cmd_upper, "To Uppercase"},
    {"rev", cmd_rev, "Reverse String"},
    // Utils
    {"clear", cmd_clear, "Clear Screen"},
    {"cls", cmd_clear, "Alias for clear"},
    {"echo", cmd_echo, "Print text"},
    {"time", cmd_time, "Show Time"},
    {"date", cmd_date, "Show Date"},
    {"color", cmd_color, "Set Text Color"},
    {"calc", cmd_calc, "Basic Calculator"},
    {"tree", cmd_tree, "Show Tree"},
    // Fun
    {"weather", cmd_weather, "Sim Weather"},
    {"rand", cmd_rand, "Random Number"},
    {"dice", cmd_dice, "Roll D6"},
    {"beep", cmd_beep, "System Beep"},
    {"selfdestruct", cmd_selfdestruct, "Don't run this"},
    {"run", cmd_run, "Run System Command"},
    {"history", cmd_history, "Command History"}, 
    {"exit", cmd_exit, "Shutdown"},
    {"shutdown", cmd_exit, "Shutdown"},
    {"help", cmd_help, "List Commands"}
};


void cmd_help(char** args, int c) {
    print_header("HELP MENU");
    int count = sizeof(commands) / sizeof(Command);
    for(int i=0; i<count; i++) {
        set_col(C_INFO);
        printf("%-10s", commands[i].name);
        set_col(C_RESET);
        printf(" : %s\n", commands[i].desc);
    }
}

// --- MAIN LOOP ---
int main() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    srand(time(NULL));
    update_cwd();

    set_col(C_OK);
    printf("TNRM1N4L v%s initialized.\n", VERSION);
    set_col(C_RESET);
    printf("Type 'help' for commands.\n");

    char input[MAX_CMD_LEN];
    char* args[MAX_ARGS];

    while (running) {
        set_col(C_OK);
        printf("\n%s > ", current_dir);
        set_col(C_RESET);

        if (!fgets(input, MAX_CMD_LEN, stdin)) break;
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;

        // Tokenize
        int arg_c = 0;
        char* token = strtok(input, " ");
        while(token && arg_c < MAX_ARGS) {
            args[arg_c++] = token;
            token = strtok(NULL, " ");
        }

        if (arg_c == 0) continue;

        // Dispatch
        int found = 0;
        int cmd_count = sizeof(commands) / sizeof(Command);
        
        for(int i=0; i<cmd_count; i++) {
            if (strcmp(args[0], commands[i].name) == 0) {
                commands[i].func(args, arg_c);
                found = 1;
                break;
            }
        }

        if (!found) {
            set_col(C_ERR);
            printf("Unknown command: %s\n", args[0]);
            set_col(C_RESET);
        }
    }
    return 0;
}
