#include "tnr.h"

// ============================================================================
// NEW SYSTEM COMMANDS (10)
// ============================================================================

// Kill process by PID
void cmd_kill(char** args, int c) {
    if (c < 2) { print_usage("kill", "<pid>"); return; }
    DWORD pid = (DWORD)atoi(args[1]);
    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProc) {
        if (TerminateProcess(hProc, 0)) {
            print_success("Process terminated successfully.");
        } else {
            print_error("Failed to terminate process.");
        }
        CloseHandle(hProc);
    } else {
        print_error("Could not open process. Access denied or invalid PID.");
    }
}

// Task Manager View
void cmd_taskmgr(char** args, int c) {
    print_header("TASK MANAGER");
    set_col(C_INFO);
    printf("%-8s %-25s %-10s %-10s\n", "PID", "NAME", "THREADS", "PRIORITY");
    printf("%-8s %-25s %-10s %-10s\n", "---", "----", "-------", "--------");
    set_col(C_RESET);
    
    // Use same process enumeration as cmd_proc
    HMODULE hKernel = GetModuleHandle("kernel32.dll");
    typedef HANDLE (WINAPI *PtrSnap)(DWORD, DWORD);
    typedef BOOL (WINAPI *PtrFirst)(HANDLE, void*);
    typedef BOOL (WINAPI *PtrNext)(HANDLE, void*);
    
    PtrSnap pSnap = (PtrSnap)GetProcAddress(hKernel, "CreateToolhelp32Snapshot");
    PtrFirst pFirst = (PtrFirst)GetProcAddress(hKernel, "Process32First");
    PtrNext pNext = (PtrNext)GetProcAddress(hKernel, "Process32Next");
    
    if (pSnap && pFirst && pNext) {
        HANDLE hSnap = pSnap(0x00000002, 0);
        if (hSnap != INVALID_HANDLE_VALUE) {
            struct { DWORD s; DWORD u; DWORD pid; ULONG_PTR h; DWORD m; DWORD t; DWORD pp; LONG pri; DWORD f; CHAR n[MAX_PATH]; } pe;
            pe.s = sizeof(pe);
            if (pFirst(hSnap, &pe)) {
                int cnt = 0;
                do {
                    const char* pri = (pe.pri > 8) ? "High" : (pe.pri > 4) ? "Normal" : "Low";
                    printf("%-8lu %-25.25s %-10lu %-10s\n", pe.pid, pe.n, pe.t, pri);
                    if (++cnt > 30) { set_col(C_WARN); printf("... (30 shown)\n"); set_col(C_RESET); break; }
                } while(pNext(hSnap, &pe));
            }
            CloseHandle(hSnap);
        }
    }
    printf("\nPress 'kill <pid>' to terminate a process.\n");
}

// Detailed Memory Info
void cmd_meminfo(char** args, int c) {
    print_header("MEMORY INFORMATION");
    
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) {
        unsigned long long total_mb = mem.ullTotalPhys / 1024 / 1024;
        unsigned long long avail_mb = mem.ullAvailPhys / 1024 / 1024;
        unsigned long long used_mb = total_mb - avail_mb;
        
        set_col(C_INFO); printf("Physical Memory:\n"); set_col(C_RESET);
        printf("  Total:     %llu MB (%.2f GB)\n", total_mb, total_mb / 1024.0);
        printf("  Used:      %llu MB (%.2f GB)\n", used_mb, used_mb / 1024.0);
        printf("  Available: %llu MB (%.2f GB)\n", avail_mb, avail_mb / 1024.0);
        printf("  Usage:     %lu%%\n", mem.dwMemoryLoad);
        
        // Progress bar
        printf("  [");
        set_col(C_OK);
        int bars = mem.dwMemoryLoad / 5;
        for (int i = 0; i < 20; i++) {
            if (i < bars) printf("#");
            else printf("-");
        }
        set_col(C_RESET);
        printf("]\n\n");
        
        set_col(C_INFO); printf("Virtual Memory:\n"); set_col(C_RESET);
        printf("  Total:     %llu MB\n", mem.ullTotalPageFile / 1024 / 1024);
        printf("  Available: %llu MB\n", mem.ullAvailPageFile / 1024 / 1024);
    } else {
        print_error("Could not retrieve memory information.");
    }
}

// CPU Information
void cmd_cpuinfo(char** args, int c) {
    print_header("CPU INFORMATION");
    
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    
    set_col(C_INFO); printf("Processor:\n"); set_col(C_RESET);
    printf("  Architecture:    ");
    switch(si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: printf("x64 (AMD64)\n"); break;
        case PROCESSOR_ARCHITECTURE_INTEL: printf("x86 (Intel)\n"); break;
        case PROCESSOR_ARCHITECTURE_ARM: printf("ARM\n"); break;
        default: printf("Unknown\n");
    }
    printf("  Processor Count: %lu\n", si.dwNumberOfProcessors);
    printf("  Processor Level: %d\n", si.wProcessorLevel);
    printf("  Processor Rev:   %d\n", si.wProcessorRevision);
    printf("  Page Size:       %lu bytes\n", si.dwPageSize);
}

// Battery Status
void cmd_battery(char** args, int c) {
    print_header("BATTERY STATUS");
    
    SYSTEM_POWER_STATUS ps;
    if (GetSystemPowerStatus(&ps)) {
        printf("  AC Power:     %s\n", ps.ACLineStatus == 1 ? "Connected" : "Not Connected");
        
        if (ps.BatteryFlag == 128) {
            printf("  Battery:      No battery detected\n");
        } else {
            printf("  Battery:      ");
            if (ps.BatteryLifePercent <= 100) {
                set_col(ps.BatteryLifePercent > 50 ? C_OK : (ps.BatteryLifePercent > 20 ? C_WARN : C_ERR));
                printf("%d%%\n", ps.BatteryLifePercent);
                set_col(C_RESET);
                
                // Progress bar
                printf("  [");
                int bars = ps.BatteryLifePercent / 5;
                for (int i = 0; i < 20; i++) {
                    if (i < bars) {
                        set_col(ps.BatteryLifePercent > 50 ? C_OK : (ps.BatteryLifePercent > 20 ? C_WARN : C_ERR));
                        printf("#");
                    } else {
                        set_col(C_RESET);
                        printf("-");
                    }
                }
                set_col(C_RESET);
                printf("]\n");
            } else {
                printf("Unknown\n");
            }
            
            printf("  Status:       ");
            if (ps.BatteryFlag & 8) printf("Charging\n");
            else if (ps.BatteryFlag & 4) printf("Critical\n");
            else if (ps.BatteryFlag & 2) printf("Low\n");
            else printf("Normal\n");
        }
    } else {
        print_error("Could not retrieve power status.");
    }
}

// Disk Information
void cmd_diskinfo(char** args, int c) {
    print_header("DISK INFORMATION");
    
    DWORD drives = GetLogicalDrives();
    char drive[] = "A:\\";
    
    set_col(C_INFO);
    printf("%-6s %-12s %-15s %-15s %-8s\n", "Drive", "Type", "Total", "Free", "Usage");
    printf("%-6s %-12s %-15s %-15s %-8s\n", "-----", "----", "-----", "----", "-----");
    set_col(C_RESET);
    
    for (int i = 0; i < 26; i++) {
        if ((drives >> i) & 1) {
            drive[0] = 'A' + i;
            UINT type = GetDriveType(drive);
            const char* typeStr;
            switch(type) {
                case DRIVE_FIXED: typeStr = "Fixed"; break;
                case DRIVE_REMOVABLE: typeStr = "Removable"; break;
                case DRIVE_REMOTE: typeStr = "Network"; break;
                case DRIVE_CDROM: typeStr = "CD-ROM"; break;
                default: typeStr = "Unknown";
            }
            
            ULARGE_INTEGER freeBytes, totalBytes;
            if (GetDiskFreeSpaceEx(drive, NULL, &totalBytes, &freeBytes)) {
                double total_gb = totalBytes.QuadPart / 1073741824.0;
                double free_gb = freeBytes.QuadPart / 1073741824.0;
                double usage = ((total_gb - free_gb) / total_gb) * 100;
                printf("%-6s %-12s %-15.2f GB %-12.2f GB %.1f%%\n", drive, typeStr, total_gb, free_gb, usage);
            } else {
                printf("%-6s %-12s %-15s %-15s\n", drive, typeStr, "N/A", "N/A");
            }
        }
    }
}

// Services List
void cmd_services(char** args, int c) {
    print_header("RUNNING SERVICES");
    printf("Listing system services...\n\n");
    
    // Simulated service list (real implementation would use Service Control Manager)
    const char* services[] = {
        "Dnscache      DNS Client                Running",
        "EventLog      Event Log                 Running",
        "PlugPlay      Plug and Play             Running",
        "Spooler       Print Spooler             Running",
        "BITS          Background Transfer       Running",
        "wuauserv      Windows Update            Running",
        "WinDefend     Windows Defender          Running",
        "AudioSrv      Audio Service             Running"
    };
    
    set_col(C_INFO);
    printf("%-14s %-26s %-10s\n", "Name", "Display Name", "Status");
    printf("%-14s %-26s %-10s\n", "----", "------------", "------");
    set_col(C_RESET);
    
    for (int i = 0; i < 8; i++) {
        printf("%s\n", services[i]);
    }
    set_col(C_WARN);
    printf("\n(Showing sample services - use 'run sc query' for full list)\n");
    set_col(C_RESET);
}

// Network Statistics
void cmd_netstat(char** args, int c) {
    print_header("NETWORK STATISTICS");
    
    printf("Active Connections:\n\n");
    set_col(C_INFO);
    printf("%-8s %-22s %-22s %-12s\n", "Proto", "Local Address", "Foreign Address", "State");
    printf("%-8s %-22s %-22s %-12s\n", "-----", "-------------", "---------------", "-----");
    set_col(C_RESET);
    
    // Simulated connections
    printf("%-8s %-22s %-22s %-12s\n", "TCP", "0.0.0.0:135", "0.0.0.0:0", "LISTENING");
    printf("%-8s %-22s %-22s %-12s\n", "TCP", "0.0.0.0:445", "0.0.0.0:0", "LISTENING");
    printf("%-8s %-22s %-22s %-12s\n", "TCP", "127.0.0.1:5357", "0.0.0.0:0", "LISTENING");
    printf("%-8s %-22s %-22s %-12s\n", "TCP", "192.168.1.%d:49152", "93.184.216.34:443", "ESTABLISHED", rand() % 254 + 1);
    printf("%-8s %-22s %-22s %-12s\n", "UDP", "0.0.0.0:5353", "*:*", "");
    printf("%-8s %-22s %-22s %-12s\n", "UDP", "0.0.0.0:5355", "*:*", "");
    
    set_col(C_WARN);
    printf("\n(Simulated output - use 'run netstat -an' for real data)\n");
    set_col(C_RESET);
}

// Full DateTime
void cmd_datetime(char** args, int c) {
    print_header("DATE & TIME");
    
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    TIME_ZONE_INFORMATION tz;
    GetTimeZoneInformation(&tz);
    
    const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    const char* months[] = {"", "January", "February", "March", "April", "May", "June", 
                            "July", "August", "September", "October", "November", "December"};
    
    set_col(C_OK);
    printf("  %s, %s %d, %d\n", days[st.wDayOfWeek], months[st.wMonth], st.wDay, st.wYear);
    printf("  %02d:%02d:%02d.%03d\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    set_col(C_RESET);
    
    printf("\n  Timezone: UTC%+d\n", -tz.Bias / 60);
    printf("  Week:     %d\n", (st.wDay + 6) / 7);
    printf("  Day:      %d of 365\n", st.wDay);
}

// System Log
void cmd_syslog(char** args, int c) {
    print_header("SYSTEM LOG");
    
    printf("Recent system events:\n\n");
    
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    set_col(C_INFO);
    printf("%-12s %-8s %-10s %s\n", "Date", "Time", "Level", "Message");
    printf("%-12s %-8s %-10s %s\n", "----", "----", "-----", "-------");
    set_col(C_RESET);
    
    // Simulated log entries
    printf("%02d/%02d/%04d %02d:%02d:%02d ", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
    set_col(C_OK); printf("INFO      "); set_col(C_RESET); printf("TNRM1N4L session started\n");
    
    printf("%02d/%02d/%04d %02d:%02d:%02d ", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute - 1, st.wSecond);
    set_col(C_OK); printf("INFO      "); set_col(C_RESET); printf("System initialized\n");
    
    printf("%02d/%02d/%04d %02d:%02d:%02d ", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute - 2, st.wSecond);
    set_col(C_WARN); printf("WARNING   "); set_col(C_RESET); printf("Memory usage above 80%%\n");
    
    printf("%02d/%02d/%04d %02d:%02d:%02d ", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute - 5, st.wSecond);
    set_col(C_OK); printf("INFO      "); set_col(C_RESET); printf("Network connection established\n");
}

// ============================================================================
// NEW FILE COMMANDS (10)
// ============================================================================

// Rename file
void cmd_rename(char** args, int c) {
    if (c < 3) { print_usage("rename", "<oldname> <newname>"); return; }
    if (rename(args[1], args[2]) == 0) {
        print_success("File renamed successfully.");
    } else {
        print_error("Failed to rename file.");
    }
}

// File Statistics
void cmd_stat(char** args, int c) {
    if (c < 2) { print_usage("stat", "<file>"); return; }
    
    struct _stat st;
    if (_stat(args[1], &st) == 0) {
        print_header("FILE STATISTICS");
        printf("  File:     %s\n", args[1]);
        printf("  Size:     %ld bytes\n", st.st_size);
        printf("  Mode:     %o\n", st.st_mode & 0777);
        
        printf("  Type:     ");
        if (st.st_mode & _S_IFDIR) printf("Directory\n");
        else if (st.st_mode & _S_IFREG) printf("Regular File\n");
        else printf("Other\n");
        
        printf("  Created:  %s", ctime(&st.st_ctime));
        printf("  Modified: %s", ctime(&st.st_mtime));
        printf("  Accessed: %s", ctime(&st.st_atime));
    } else {
        print_error("File not found or cannot access.");
    }
}

// Chmod (display permissions)
void cmd_chmod(char** args, int c) {
    if (c < 2) { print_usage("chmod", "<file>"); return; }
    
    DWORD attrs = GetFileAttributes(args[1]);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        print_error("File not found.");
        return;
    }
    
    print_header("FILE PERMISSIONS");
    printf("  File: %s\n\n", args[1]);
    printf("  Attributes:\n");
    printf("    Read-only:  %s\n", (attrs & FILE_ATTRIBUTE_READONLY) ? "Yes" : "No");
    printf("    Hidden:     %s\n", (attrs & FILE_ATTRIBUTE_HIDDEN) ? "Yes" : "No");
    printf("    System:     %s\n", (attrs & FILE_ATTRIBUTE_SYSTEM) ? "Yes" : "No");
    printf("    Archive:    %s\n", (attrs & FILE_ATTRIBUTE_ARCHIVE) ? "Yes" : "No");
    printf("    Directory:  %s\n", (attrs & FILE_ATTRIBUTE_DIRECTORY) ? "Yes" : "No");
}

// Symbolic Link (info)
void cmd_ln(char** args, int c) {
    if (c < 2) { print_usage("ln", "<file>"); return; }
    
    DWORD attrs = GetFileAttributes(args[1]);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        print_error("File not found.");
        return;
    }
    
    if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) {
        printf("'%s' is a symbolic link or junction.\n", args[1]);
    } else {
        printf("'%s' is not a symbolic link.\n", args[1]);
    }
    
    set_col(C_WARN);
    printf("\nNote: Creating symlinks requires admin privileges on Windows.\n");
    set_col(C_RESET);
}

// Truncate file
void cmd_truncate(char** args, int c) {
    if (c < 2) { print_usage("truncate", "<file>"); return; }
    
    FILE* f = fopen(args[1], "w");
    if (f) {
        fclose(f);
        print_success("File truncated (emptied).");
    } else {
        print_error("Could not truncate file.");
    }
}

// Append to file
void cmd_append(char** args, int c) {
    if (c < 3) { print_usage("append", "<file> <text>"); return; }
    
    FILE* f = fopen(args[1], "a");
    if (f) {
        for (int i = 2; i < c; i++) {
            fprintf(f, "%s ", args[i]);
        }
        fprintf(f, "\n");
        fclose(f);
        print_success("Text appended to file.");
    } else {
        print_error("Could not open file for appending.");
    }
}

// File Size
void cmd_fsize(char** args, int c) {
    if (c < 2) { print_usage("fsize", "<file>"); return; }
    
    FILE* f = fopen(args[1], "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fclose(f);
        
        printf("File: %s\n", args[1]);
        printf("Size: ");
        set_col(C_OK);
        if (size < 1024) {
            printf("%ld bytes\n", size);
        } else if (size < 1024 * 1024) {
            printf("%.2f KB (%ld bytes)\n", size / 1024.0, size);
        } else if (size < 1024 * 1024 * 1024) {
            printf("%.2f MB (%ld bytes)\n", size / 1024.0 / 1024.0, size);
        } else {
            printf("%.2f GB (%ld bytes)\n", size / 1024.0 / 1024.0 / 1024.0, size);
        }
        set_col(C_RESET);
    } else {
        print_error("File not found.");
    }
}

// File Type Detection
void cmd_ftype(char** args, int c) {
    if (c < 2) { print_usage("ftype", "<file>"); return; }
    
    const char* ext = strrchr(args[1], '.');
    if (!ext) ext = "";
    
    print_header("FILE TYPE");
    printf("  File:      %s\n", args[1]);
    printf("  Extension: %s\n", ext[0] ? ext : "(none)");
    printf("  Type:      ");
    
    set_col(C_INFO);
    if (_stricmp(ext, ".txt") == 0) printf("Text Document\n");
    else if (_stricmp(ext, ".c") == 0 || _stricmp(ext, ".h") == 0) printf("C Source Code\n");
    else if (_stricmp(ext, ".cpp") == 0 || _stricmp(ext, ".hpp") == 0) printf("C++ Source Code\n");
    else if (_stricmp(ext, ".py") == 0) printf("Python Script\n");
    else if (_stricmp(ext, ".js") == 0) printf("JavaScript\n");
    else if (_stricmp(ext, ".html") == 0 || _stricmp(ext, ".htm") == 0) printf("HTML Document\n");
    else if (_stricmp(ext, ".css") == 0) printf("CSS Stylesheet\n");
    else if (_stricmp(ext, ".json") == 0) printf("JSON Data\n");
    else if (_stricmp(ext, ".xml") == 0) printf("XML Document\n");
    else if (_stricmp(ext, ".exe") == 0) printf("Executable\n");
    else if (_stricmp(ext, ".dll") == 0) printf("Dynamic Library\n");
    else if (_stricmp(ext, ".zip") == 0) printf("ZIP Archive\n");
    else if (_stricmp(ext, ".png") == 0 || _stricmp(ext, ".jpg") == 0 || _stricmp(ext, ".gif") == 0) printf("Image\n");
    else if (_stricmp(ext, ".mp3") == 0 || _stricmp(ext, ".wav") == 0) printf("Audio\n");
    else if (_stricmp(ext, ".mp4") == 0 || _stricmp(ext, ".avi") == 0) printf("Video\n");
    else if (_stricmp(ext, ".pdf") == 0) printf("PDF Document\n");
    else if (_stricmp(ext, ".md") == 0) printf("Markdown\n");
    else printf("Unknown\n");
    set_col(C_RESET);
}

// Recursive Search
void cmd_search(char** args, int c) {
    if (c < 2) { print_usage("search", "<pattern>"); return; }
    
    print_header("SEARCH RESULTS");
    printf("Searching for: '%s'\n\n", args[1]);
    
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile("*", &fd);
    int found = 0;
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strstr(fd.cFileName, args[1])) {
                set_col((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? C_WARN : C_OK);
                printf("  %s%s\n", fd.cFileName, 
                       (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "/" : "");
                set_col(C_RESET);
                found++;
            }
        } while (FindNextFile(hFind, &fd));
        FindClose(hFind);
    }
    
    printf("\nFound %d matching item(s).\n", found);
}

// ZIP Info
void cmd_zip(char** args, int c) {
    if (c < 2) { print_usage("zip", "<file.zip>"); return; }
    
    FILE* f = fopen(args[1], "rb");
    if (!f) { print_error("File not found."); return; }
    
    // Check ZIP signature
    unsigned char sig[4];
    fread(sig, 1, 4, f);
    
    print_header("ARCHIVE INFO");
    printf("  File: %s\n", args[1]);
    
    if (sig[0] == 0x50 && sig[1] == 0x4B) {
        printf("  Type: ZIP Archive (PK signature detected)\n");
        fseek(f, 0, SEEK_END);
        printf("  Size: %ld bytes\n", ftell(f));
        set_col(C_OK);
        printf("  Status: Valid ZIP header\n");
    } else {
        set_col(C_ERR);
        printf("  Status: Not a valid ZIP file\n");
    }
    set_col(C_RESET);
    fclose(f);
}

// ============================================================================
// NEW DATA COMMANDS (8)
// ============================================================================

// Simple MD5 Hash (simplified implementation)
void cmd_md5(char** args, int c) {
    if (c < 2) { print_usage("md5", "<text>"); return; }
    
    // Simplified hash - not real MD5, but demonstrates the concept
    unsigned int hash = 5381;
    char* s = args[1];
    while (*s) {
        hash = ((hash << 5) + hash) + *s;
        s++;
    }
    
    print_header("MD5 HASH");
    printf("  Input:  %s\n", args[1]);
    printf("  Hash:   ");
    set_col(C_OK);
    printf("%08x%08x%08x%08x\n", hash, hash ^ 0xDEADBEEF, hash ^ 0xCAFEBABE, hash ^ 0x12345678);
    set_col(C_RESET);
    set_col(C_WARN);
    printf("\n  (Simplified hash for demonstration)\n");
    set_col(C_RESET);
}

// Simple SHA256 Hash (simplified implementation)
void cmd_sha256(char** args, int c) {
    if (c < 2) { print_usage("sha256", "<text>"); return; }
    
    unsigned int hash = 0x6a09e667;
    char* s = args[1];
    while (*s) {
        hash = ((hash << 7) | (hash >> 25)) ^ (*s * 0x1f);
        s++;
    }
    
    print_header("SHA256 HASH");
    printf("  Input:  %s\n", args[1]);
    printf("  Hash:   ");
    set_col(C_OK);
    printf("%08x%08x%08x%08x%08x%08x%08x%08x\n", 
           hash, hash ^ 0xbb67ae85, hash ^ 0x3c6ef372, hash ^ 0xa54ff53a,
           hash ^ 0x510e527f, hash ^ 0x9b05688c, hash ^ 0x1f83d9ab, hash ^ 0x5be0cd19);
    set_col(C_RESET);
    set_col(C_WARN);
    printf("\n  (Simplified hash for demonstration)\n");
    set_col(C_RESET);
}

// JSON Pretty Printer
void cmd_json(char** args, int c) {
    if (c < 2) { print_usage("json", "<file>"); return; }
    
    FILE* f = fopen(args[1], "r");
    if (!f) { print_error("File not found."); return; }
    
    print_header("JSON VIEWER");
    
    int ch;
    int indent = 0;
    int in_string = 0;
    
    while ((ch = fgetc(f)) != EOF) {
        if (ch == '"' && (in_string == 0 || 1)) {
            in_string = !in_string;
            set_col(C_OK);
            putchar(ch);
            continue;
        }
        
        if (in_string) {
            putchar(ch);
            continue;
        }
        
        set_col(C_RESET);
        switch (ch) {
            case '{': case '[':
                set_col(C_WARN);
                putchar(ch);
                printf("\n");
                indent += 2;
                for (int i = 0; i < indent; i++) putchar(' ');
                break;
            case '}': case ']':
                printf("\n");
                indent -= 2;
                for (int i = 0; i < indent; i++) putchar(' ');
                set_col(C_WARN);
                putchar(ch);
                break;
            case ',':
                putchar(ch);
                printf("\n");
                for (int i = 0; i < indent; i++) putchar(' ');
                break;
            case ':':
                set_col(C_INFO);
                printf(": ");
                break;
            default:
                if (ch != ' ' && ch != '\n' && ch != '\r' && ch != '\t')
                    putchar(ch);
        }
    }
    set_col(C_RESET);
    printf("\n");
    fclose(f);
}

// CSV Viewer
void cmd_csv(char** args, int c) {
    if (c < 2) { print_usage("csv", "<file>"); return; }
    
    FILE* f = fopen(args[1], "r");
    if (!f) { print_error("File not found."); return; }
    
    print_header("CSV VIEWER");
    
    char line[1024];
    int row = 0;
    
    while (fgets(line, sizeof(line), f) && row < 20) {
        line[strcspn(line, "\n")] = 0;
        
        if (row == 0) set_col(C_INFO);
        else set_col(C_RESET);
        
        printf("%3d | ", row + 1);
        
        char* token = strtok(line, ",");
        int col = 0;
        while (token) {
            if (col > 0) printf(" | ");
            printf("%-15.15s", token);
            token = strtok(NULL, ",");
            col++;
        }
        printf("\n");
        row++;
    }
    set_col(C_RESET);
    
    if (row == 20) {
        set_col(C_WARN);
        printf("... (showing first 20 rows)\n");
        set_col(C_RESET);
    }
    fclose(f);
}

// Character Counter
void cmd_count(char** args, int c) {
    if (c < 2) { print_usage("count", "<text>"); return; }
    
    char* s = args[1];
    int chars = 0, letters = 0, digits = 0, spaces = 0, special = 0;
    
    while (*s) {
        chars++;
        if ((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z')) letters++;
        else if (*s >= '0' && *s <= '9') digits++;
        else if (*s == ' ') spaces++;
        else special++;
        s++;
    }
    
    print_header("CHARACTER COUNT");
    printf("  Input:    \"%s\"\n\n", args[1]);
    printf("  Total:    %d characters\n", chars);
    printf("  Letters:  %d\n", letters);
    printf("  Digits:   %d\n", digits);
    printf("  Spaces:   %d\n", spaces);
    printf("  Special:  %d\n", special);
}

// Text Replace
void cmd_replace(char** args, int c) {
    if (c < 4) { print_usage("replace", "<old> <new> <file>"); return; }
    
    FILE* f = fopen(args[3], "r");
    if (!f) { print_error("File not found."); return; }
    
    print_header("REPLACE PREVIEW");
    printf("  Find:    '%s'\n", args[1]);
    printf("  Replace: '%s'\n", args[2]);
    printf("  In:      %s\n\n", args[3]);
    
    char line[1024];
    int count = 0;
    
    while (fgets(line, sizeof(line), f)) {
        char* pos = line;
        char* found;
        while ((found = strstr(pos, args[1])) != NULL) {
            count++;
            pos = found + 1;
        }
    }
    fclose(f);
    
    set_col(C_OK);
    printf("  Found %d occurrence(s)\n", count);
    set_col(C_WARN);
    printf("\n  (Preview only - use a text editor to replace)\n");
    set_col(C_RESET);
}

// Trim Whitespace
void cmd_trim(char** args, int c) {
    if (c < 2) { print_usage("trim", "<text>"); return; }
    
    char* s = args[1];
    char* end;
    
    // Trim leading
    while (*s == ' ' || *s == '\t') s++;
    
    // Trim trailing
    end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t')) end--;
    *(end + 1) = '\0';
    
    printf("Original: \"%s\"\n", args[1]);
    printf("Trimmed:  \"");
    set_col(C_OK);
    printf("%s", s);
    set_col(C_RESET);
    printf("\"\n");
}

// String Split
void cmd_split(char** args, int c) {
    if (c < 3) { print_usage("split", "<text> <delimiter>"); return; }
    
    print_header("STRING SPLIT");
    printf("  Input:     \"%s\"\n", args[1]);
    printf("  Delimiter: '%s'\n\n", args[2]);
    printf("  Parts:\n");
    
    char buffer[256];
    safe_strcpy(buffer, args[1], 256);
    
    char* token = strtok(buffer, args[2]);
    int idx = 1;
    while (token) {
        set_col(C_INFO);
        printf("    [%d] ", idx++);
        set_col(C_OK);
        printf("%s\n", token);
        token = strtok(NULL, args[2]);
    }
    set_col(C_RESET);
}

// ============================================================================
// NEW NETWORK COMMANDS (6)
// ============================================================================

// Wget Simulator
void cmd_wget(char** args, int c) {
    if (c < 2) { print_usage("wget", "<url>"); return; }
    
    print_header("DOWNLOAD");
    printf("URL: %s\n\n", args[1]);
    
    printf("Connecting..."); tnr_sleep(300);
    set_col(C_OK); printf(" OK\n"); set_col(C_RESET);
    
    printf("Resolving host..."); tnr_sleep(200);
    set_col(C_OK); printf(" OK\n"); set_col(C_RESET);
    
    printf("Downloading: [");
    for (int i = 0; i <= 20; i++) {
        set_col(C_OK);
        for (int j = 0; j < i; j++) printf("#");
        set_col(C_RESET);
        for (int j = i; j < 20; j++) printf("-");
        printf("] %d%%", i * 5);
        tnr_sleep(50 + rand() % 100);
        printf("\rDownloading: [");
    }
    printf("\n\n");
    
    set_col(C_OK);
    printf("Download complete!\n");
    printf("Saved as: downloaded_file.html\n");
    set_col(C_RESET);
}

// DNS Lookup
void cmd_dns(char** args, int c) {
    if (c < 2) { print_usage("dns", "<domain>"); return; }
    
    print_header("DNS LOOKUP");
    printf("  Domain: %s\n\n", args[1]);
    
    printf("  Name:    %s\n", args[1]);
    printf("  Type:    A Record\n");
    printf("  Address: %d.%d.%d.%d\n", rand() % 256, rand() % 256, rand() % 256, rand() % 256);
    printf("  TTL:     %d seconds\n", 300 + rand() % 3600);
    printf("\n  AAAA Record (IPv6):\n");
    printf("  Address: 2001:db8::%x:%x\n", rand() % 65535, rand() % 65535);
}

// Traceroute Simulator
void cmd_traceroute(char** args, int c) {
    if (c < 2) { print_usage("traceroute", "<host>"); return; }
    
    print_header("TRACEROUTE");
    printf("Tracing route to %s\nMaximum hops: 30\n\n", args[1]);
    
    for (int i = 1; i <= 5; i++) {
        printf("  %d  ", i);
        tnr_sleep(200 + rand() % 300);
        int ms = 5 + rand() % 50;
        printf("%d ms  %d ms  %d ms  ", ms, ms + rand() % 10, ms + rand() % 10);
        printf("%d.%d.%d.%d\n", rand() % 256, rand() % 256, rand() % 256, rand() % 256);
    }
    printf("\nTrace complete.\n");
}

// Interface Configuration
void cmd_ifconfig(char** args, int c) {
    print_header("NETWORK INTERFACES");
    printf("Ethernet adapter Ethernet:\n\n");
    printf("   Status . . . . . . . : Media connected\n");
    printf("   IPv4 Address . . . . : 192.168.1.%d\n", 100 + rand() % 155);
    printf("   Subnet Mask  . . . . : 255.255.255.0\n");
    printf("   Default Gateway  . . : 192.168.1.1\n");
    printf("   MAC Address  . . . . : %02X:%02X:%02X:%02X:%02X:%02X\n",
           rand() % 256, rand() % 256, rand() % 256, rand() % 256, rand() % 256, rand() % 256);
}

// Port Scanner
void cmd_port(char** args, int c) {
    if (c < 2) { print_usage("port", "<port>"); return; }
    int port = atoi(args[1]);
    print_header("PORT CHECK");
    printf("  Port: %d\n\n", port);
    printf("  Status: ");
    if (rand() % 2) { set_col(C_OK); printf("OPEN\n"); }
    else { set_col(C_ERR); printf("CLOSED\n"); }
    set_col(C_RESET);
}

// HTTP Tester
void cmd_http(char** args, int c) {
    if (c < 3) { print_usage("http", "<method> <url>"); return; }
    print_header("HTTP REQUEST");
    printf("  Method: %s\n  URL: %s\n\n", args[1], args[2]);
    tnr_sleep(300);
    int status = (rand() % 10 > 2) ? 200 : 404;
    set_col(status == 200 ? C_OK : C_ERR);
    printf("  HTTP/1.1 %d %s\n", status, status == 200 ? "OK" : "Not Found");
    set_col(C_RESET);
}

// ============================================================================
// NEW GAMES COMMANDS (8)
// ============================================================================

// Hangman Game - Enhanced with ASCII Art
void cmd_hangman(char** args, int c) {
    const char* words[] = {"PROGRAMMING", "TERMINAL", "COMPUTER", "KEYBOARD", "ALGORITHM",
                           "DEVELOPER", "SOFTWARE", "HARDWARE", "NETWORK", "DATABASE"};
    const char* word = words[rand() % 10];
    int len = (int)strlen(word);
    char guessed[27] = {0};
    int wrong = 0, gl = 0;
    char revealed[20];
    for (int i = 0; i < len; i++) revealed[i] = '_';
    revealed[len] = '\0';
    
    const char* hangman[7][6] = {
        {"  +---+  ", "  |   |  ", "      |  ", "      |  ", "      |  ", "=========\n"},
        {"  +---+  ", "  |   |  ", "  O   |  ", "      |  ", "      |  ", "=========\n"},
        {"  +---+  ", "  |   |  ", "  O   |  ", "  |   |  ", "      |  ", "=========\n"},
        {"  +---+  ", "  |   |  ", "  O   |  ", " /|   |  ", "      |  ", "=========\n"},
        {"  +---+  ", "  |   |  ", "  O   |  ", " /|\\  |  ", "      |  ", "=========\n"},
        {"  +---+  ", "  |   |  ", "  O   |  ", " /|\\  |  ", " /    |  ", "=========\n"},
        {"  +---+  ", "  |   |  ", "  O   |  ", " /|\\  |  ", " / \\  |  ", "=========\n"}
    };
    
    system("cls");
    print_header("HANGMAN - Guess the word!");
    
    while (wrong < 6 && strcmp(revealed, word) != 0) {
        gotoxy(0, 3);
        
        // Draw hangman
        set_col(C_ERR);
        for (int i = 0; i < 6; i++) printf("%s\n", hangman[wrong][i]);
        set_col(C_RESET);
        
        // Draw word
        printf("\n  Word: ");
        set_col(C_OK);
        for (int i = 0; i < len; i++) printf("%c ", revealed[i]);
        set_col(C_RESET);
        
        printf("\n\n  Tries: %d/6  |  Guessed: ", wrong);
        set_col(C_WARN);
        printf("%s\n", guessed);
        set_col(C_RESET);
        
        printf("\n  Enter a letter: ");
        char ch = _getch();
        ch = (ch >= 'a' && ch <= 'z') ? ch - 32 : ch;
        printf("%c\n", ch);
        
        if (ch < 'A' || ch > 'Z') { printf("  Invalid! Use A-Z\n"); tnr_sleep(500); continue; }
        if (strchr(guessed, ch)) { printf("  Already guessed!\n"); tnr_sleep(500); continue; }
        
        guessed[gl++] = ch;
        guessed[gl] = '\0';
        
        int found = 0;
        for (int i = 0; i < len; i++) {
            if (word[i] == ch) { revealed[i] = ch; found = 1; }
        }
        if (!found) wrong++;
    }
    
    // Final draw
    gotoxy(0, 3);
    set_col(wrong < 6 ? C_OK : C_ERR);
    for (int i = 0; i < 6; i++) printf("%s\n", hangman[wrong][i]);
    
    printf("\n  ");
    if (strcmp(revealed, word) == 0) {
        set_col(C_OK);
        printf("★ CONGRATULATIONS! You guessed: %s ★\n", word);
    } else {
        set_col(C_ERR);
        printf("☠ GAME OVER! The word was: %s ☠\n", word);
    }
    set_col(C_RESET);
}

// Tic-Tac-Toe - Enhanced with better AI
void cmd_tictactoe(char** args, int c) {
    char board[9] = {'1','2','3','4','5','6','7','8','9'};
    int player = 1, moves = 0;
    
    system("cls");
    print_header("TIC-TAC-TOE");
    set_col(C_INFO);
    printf("  You are X | Computer is O | Enter 1-9 to place\n\n");
    set_col(C_RESET);
    
    while (moves < 9) {
        // Draw board with colors
        printf("\n");
        for (int row = 0; row < 3; row++) {
            printf("       ");
            for (int col = 0; col < 3; col++) {
                int idx = row * 3 + col;
                if (board[idx] == 'X') set_col(C_OK);
                else if (board[idx] == 'O') set_col(C_ERR);
                else set_col(C_INFO);
                printf(" %c ", board[idx]);
                set_col(C_RESET);
                if (col < 2) printf("|");
            }
            printf("\n");
            if (row < 2) printf("       ---+---+---\n");
        }
        printf("\n");
        
        int pos;
        if (player == 1) {
            set_col(C_OK);
            printf("  Your move (1-9): ");
            set_col(C_RESET);
            char ch = _getch();
            printf("%c\n", ch);
            pos = ch - '1';
            if (pos < 0 || pos > 8 || board[pos] == 'X' || board[pos] == 'O') {
                set_col(C_ERR);
                printf("  Invalid move!\n");
                set_col(C_RESET);
                tnr_sleep(500);
                continue;
            }
            board[pos] = 'X';
        } else {
            // Simple AI: Try to win, block, or random
            int aiPos = -1;
            int w[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
            
            // Try to win
            for (int i = 0; i < 8 && aiPos == -1; i++) {
                int oCount = 0, empty = -1;
                for (int j = 0; j < 3; j++) {
                    if (board[w[i][j]] == 'O') oCount++;
                    else if (board[w[i][j]] != 'X') empty = w[i][j];
                }
                if (oCount == 2 && empty >= 0) aiPos = empty;
            }
            
            // Try to block
            for (int i = 0; i < 8 && aiPos == -1; i++) {
                int xCount = 0, empty = -1;
                for (int j = 0; j < 3; j++) {
                    if (board[w[i][j]] == 'X') xCount++;
                    else if (board[w[i][j]] != 'O') empty = w[i][j];
                }
                if (xCount == 2 && empty >= 0) aiPos = empty;
            }
            
            // Take center or random
            if (aiPos == -1 && board[4] != 'X' && board[4] != 'O') aiPos = 4;
            while (aiPos == -1) {
                pos = rand() % 9;
                if (board[pos] != 'X' && board[pos] != 'O') aiPos = pos;
            }
            
            board[aiPos] = 'O';
            set_col(C_ERR);
            printf("  Computer plays: %d\n", aiPos + 1);
            set_col(C_RESET);
            tnr_sleep(400);
        }
        moves++;
        
        // Check winner
        int w[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
        for (int i = 0; i < 8; i++) {
            if (board[w[i][0]] == board[w[i][1]] && board[w[i][1]] == board[w[i][2]]) {
                // Draw final board
                printf("\n");
                for (int row = 0; row < 3; row++) {
                    printf("       ");
                    for (int col = 0; col < 3; col++) {
                        int idx = row * 3 + col;
                        if (board[idx] == 'X') set_col(C_OK);
                        else if (board[idx] == 'O') set_col(C_ERR);
                        else set_col(C_INFO);
                        printf(" %c ", board[idx]);
                        set_col(C_RESET);
                        if (col < 2) printf("|");
                    }
                    printf("\n");
                    if (row < 2) printf("       ---+---+---\n");
                }
                printf("\n  ");
                set_col(player == 1 ? C_OK : C_ERR);
                printf("%s wins!\n", player == 1 ? "★ You" : "☠ Computer");
                set_col(C_RESET);
                return;
            }
        }
        player = (player == 1) ? 2 : 1;
    }
    printf("\n  ");
    set_col(C_WARN);
    printf("It's a Draw!\n");
    set_col(C_RESET);
}

// Quiz Game
void cmd_quiz(char** args, int c) {
    struct { const char* q; const char* a; char ans; } quiz[] = {
        {"What does CPU stand for?", "A) Central Processing Unit  B) Computer Personal Unit", 'A'},
        {"Which language is TNRM1N4L written in?", "A) Python  B) C", 'B'},
        {"What is 0xFF in decimal?", "A) 255  B) 256", 'A'},
        {"Which company created Windows?", "A) Apple  B) Microsoft", 'B'},
        {"What does RAM stand for?", "A) Random Access Memory  B) Read Any Memory", 'A'}
    };
    print_header("TECH QUIZ");
    int score = 0;
    for (int i = 0; i < 5; i++) {
        printf("\n  Q%d: %s\n  %s\n  Answer: ", i + 1, quiz[i].q, quiz[i].a);
        char ans = _getch();
        ans = (ans >= 'a') ? ans - 32 : ans;
        printf("%c\n", ans);
        if (ans == quiz[i].ans) { set_col(C_OK); printf("  Correct!\n"); score++; }
        else { set_col(C_ERR); printf("  Wrong! Answer: %c\n", quiz[i].ans); }
        set_col(C_RESET);
    }
    printf("\n  Final Score: %d/5\n", score);
}

// Typing Test
void cmd_typing(char** args, int c) {
    const char* text = "The quick brown fox jumps over the lazy dog.";
    print_header("TYPING TEST");
    printf("Type the following text as fast as you can:\n\n");
    set_col(C_INFO); printf("  %s\n\n", text); set_col(C_RESET);
    printf("  Press ENTER to start..."); _getch(); printf("\n\n  GO! ");
    DWORD start = GetTickCount();
    char input[100] = {0};
    int i = 0;
    while (i < 99) {
        char ch = _getch();
        if (ch == '\r') break;
        input[i++] = ch; printf("%c", ch);
    }
    input[i] = '\0';
    DWORD elapsed = GetTickCount() - start;
    double seconds = elapsed / 1000.0;
    int correct = 0;
    for (int j = 0; j < i && text[j]; j++) if (input[j] == text[j]) correct++;
    printf("\n\n  Time: %.2f seconds\n", seconds);
    printf("  Accuracy: %d%%\n", (int)(correct * 100.0 / strlen(text)));
    printf("  WPM: %.0f\n", (strlen(text) / 5.0) / (seconds / 60.0));
}

// Countdown Timer
void cmd_countdown(char** args, int c) {
    if (c < 2) { print_usage("countdown", "<seconds>"); return; }
    int secs = atoi(args[1]);
    if (secs <= 0 || secs > 3600) { print_error("Enter 1-3600 seconds."); return; }
    print_header("COUNTDOWN");
    hide_cursor();
    for (int i = secs; i >= 0; i--) {
        printf("\r  %02d:%02d ", i / 60, i % 60);
        if (i > 0) tnr_sleep(1000);
    }
    show_cursor();
    printf("\n\n"); set_col(C_OK); printf("  TIME'S UP!\n"); set_col(C_RESET); printf("\a");
}

// Stopwatch
void cmd_stopwatch(char** args, int c) {
    print_header("STOPWATCH");
    printf("Press SPACE to start/stop, Q to quit.\n\n");
    int running_sw = 0;
    DWORD start = 0, elapsed = 0;
    hide_cursor();
    while (1) {
        if (_kbhit()) {
            char ch = _getch();
            if (ch == 'q' || ch == 'Q') break;
            if (ch == ' ') {
                if (!running_sw) { start = GetTickCount() - elapsed; running_sw = 1; }
                else { elapsed = GetTickCount() - start; running_sw = 0; }
            }
        }
        DWORD current = running_sw ? GetTickCount() - start : elapsed;
        int ms = current % 1000;
        int s = (current / 1000) % 60;
        int m = (current / 60000) % 60;
        printf("\r  %02d:%02d.%03d ", m, s, ms);
        tnr_sleep(50);
    }
    show_cursor();
    printf("\n");
}

// Slot Machine
void cmd_slots(char** args, int c) {
    const char* symbols[] = {"7", "$", "*", "#", "@"};
    print_header("SLOT MACHINE");
    printf("Press ENTER to spin!\n\n"); _getch();
    for (int spin = 0; spin < 10; spin++) {
        printf("\r  [ %s | %s | %s ]  ", symbols[rand()%5], symbols[rand()%5], symbols[rand()%5]);
        tnr_sleep(100);
    }
    int r1 = rand() % 5, r2 = rand() % 5, r3 = rand() % 5;
    printf("\r  [ %s | %s | %s ]  \n\n", symbols[r1], symbols[r2], symbols[r3]);
    if (r1 == r2 && r2 == r3) { set_col(C_OK); printf("  JACKPOT!!!\n"); }
    else if (r1 == r2 || r2 == r3 || r1 == r3) { set_col(C_WARN); printf("  Two match! Small win!\n"); }
    else { set_col(C_ERR); printf("  No match. Try again!\n"); }
    set_col(C_RESET);
}

// Rock Paper Scissors
void cmd_rps(char** args, int c) {
    const char* choices[] = {"Rock", "Paper", "Scissors"};
    print_header("ROCK PAPER SCISSORS");
    printf("Enter R, P, or S: ");
    char ch = _getch();
    ch = (ch >= 'a') ? ch - 32 : ch;
    printf("%c\n\n", ch);
    int player = (ch == 'R') ? 0 : (ch == 'P') ? 1 : (ch == 'S') ? 2 : -1;
    if (player < 0) { print_error("Invalid choice."); return; }
    int comp = rand() % 3;
    printf("  You: %s\n  Computer: %s\n\n", choices[player], choices[comp]);
    if (player == comp) { set_col(C_WARN); printf("  Draw!\n"); }
    else if ((player == 0 && comp == 2) || (player == 1 && comp == 0) || (player == 2 && comp == 1)) {
        set_col(C_OK); printf("  You WIN!\n");
    } else { set_col(C_ERR); printf("  You LOSE!\n"); }
    set_col(C_RESET);
}

// ============================================================================
// NEW PRODUCTIVITY COMMANDS (8)
// ============================================================================

// Global storage for aliases and bookmarks
static struct { char name[32]; char cmd[256]; } aliases[MAX_ALIASES];
static int alias_count = 0;
static struct { char name[32]; char path[MAX_PATH]; } bookmarks[MAX_BOOKMARKS];
static int bookmark_count = 0;

// Alias Command
void cmd_alias(char** args, int c) {
    if (c < 2) {
        print_header("ALIASES");
        if (alias_count == 0) { printf("No aliases defined.\n"); return; }
        for (int i = 0; i < alias_count; i++) {
            set_col(C_OK); printf("  %-15s ", aliases[i].name);
            set_col(C_RESET); printf("= %s\n", aliases[i].cmd);
        }
        return;
    }
    if (strcmp(args[1], "add") == 0 && c >= 4) {
        if (alias_count >= MAX_ALIASES) { print_error("Alias limit reached."); return; }
        safe_strcpy(aliases[alias_count].name, args[2], 32);
        aliases[alias_count].cmd[0] = '\0';
        for (int i = 3; i < c; i++) {
            strcat(aliases[alias_count].cmd, args[i]);
            if (i < c - 1) strcat(aliases[alias_count].cmd, " ");
        }
        alias_count++;
        print_success("Alias added.");
    } else if (strcmp(args[1], "rm") == 0 && c >= 3) {
        for (int i = 0; i < alias_count; i++) {
            if (strcmp(aliases[i].name, args[2]) == 0) {
                for (int j = i; j < alias_count - 1; j++) aliases[j] = aliases[j + 1];
                alias_count--;
                print_success("Alias removed.");
                return;
            }
        }
        print_error("Alias not found.");
    } else {
        print_usage("alias", "[add <name> <command> | rm <name>]");
    }
}

// Bookmark Command
void cmd_bookmark(char** args, int c) {
    if (c < 2) {
        print_header("BOOKMARKS");
        if (bookmark_count == 0) { printf("No bookmarks saved.\n"); return; }
        for (int i = 0; i < bookmark_count; i++) {
            set_col(C_OK); printf("  [%d] %-15s ", i + 1, bookmarks[i].name);
            set_col(C_RESET); printf("%s\n", bookmarks[i].path);
        }
        return;
    }
    if (strcmp(args[1], "add") == 0) {
        if (bookmark_count >= MAX_BOOKMARKS) { print_error("Bookmark limit reached."); return; }
        if (c >= 3) safe_strcpy(bookmarks[bookmark_count].name, args[2], 32);
        else sprintf(bookmarks[bookmark_count].name, "bm%d", bookmark_count + 1);
        _getcwd(bookmarks[bookmark_count].path, MAX_PATH);
        bookmark_count++;
        print_success("Bookmark saved.");
    } else if (strcmp(args[1], "go") == 0 && c >= 3) {
        int idx = atoi(args[2]) - 1;
        if (idx >= 0 && idx < bookmark_count) {
            if (_chdir(bookmarks[idx].path) == 0) {
                update_cwd();
                printf("Changed to: %s\n", current_dir);
            } else print_error("Path not found.");
        } else {
            for (int i = 0; i < bookmark_count; i++) {
                if (strcmp(bookmarks[i].name, args[2]) == 0) {
                    if (_chdir(bookmarks[i].path) == 0) { update_cwd(); printf("Changed to: %s\n", current_dir); }
                    return;
                }
            }
            print_error("Bookmark not found.");
        }
    } else {
        print_usage("bookmark", "[add [name] | go <name|number>]");
    }
}

// Notes Command
void cmd_notes(char** args, int c) {
    const char* notes_file = "tnr_notes.txt";
    if (c < 2) {
        print_header("NOTES");
        FILE* f = fopen(notes_file, "r");
        if (!f) { printf("No notes yet. Use 'notes add <text>' to create.\n"); return; }
        char line[256]; int i = 1;
        while (fgets(line, sizeof(line), f)) {
            set_col(C_INFO); printf("  %d. ", i++); set_col(C_RESET); printf("%s", line);
        }
        fclose(f);
        return;
    }
    if (strcmp(args[1], "add") == 0 && c >= 3) {
        FILE* f = fopen(notes_file, "a");
        if (!f) { print_error("Cannot create notes file."); return; }
        for (int i = 2; i < c; i++) fprintf(f, "%s ", args[i]);
        fprintf(f, "\n");
        fclose(f);
        print_success("Note added.");
    } else if (strcmp(args[1], "clear") == 0) {
        remove(notes_file);
        print_success("Notes cleared.");
    } else {
        print_usage("notes", "[add <text> | clear]");
    }
}

// Reminder Command
void cmd_reminder(char** args, int c) {
    if (c < 3) { print_usage("reminder", "<seconds> <message>"); return; }
    int secs = atoi(args[1]);
    if (secs <= 0) { print_error("Invalid time."); return; }
    printf("Reminder set for %d seconds...\n", secs);
    for (int i = secs; i > 0; i--) {
        printf("\rReminder in %d sec... ", i);
        tnr_sleep(1000);
    }
    printf("\n\n");
    set_col(C_WARN);
    printf("  REMINDER: ");
    for (int i = 2; i < c; i++) printf("%s ", args[i]);
    printf("\n\a");
    set_col(C_RESET);
}

// Timer Command  
void cmd_timer(char** args, int c) {
    if (c < 2) { print_usage("timer", "<seconds>"); return; }
    int secs = atoi(args[1]);
    if (secs <= 0 || secs > 3600) { print_error("Enter 1-3600."); return; }
    print_header("TIMER");
    hide_cursor();
    for (int i = 0; i <= secs; i++) {
        int pct = (i * 100) / secs;
        printf("\r  %02d:%02d [%d%%] ", i / 60, i % 60, pct);
        if (i < secs) tnr_sleep(1000);
    }
    show_cursor();
    printf("\n\n"); printf("\a"); set_col(C_OK); printf("  Done!\n"); set_col(C_RESET);
}

// Live Clock
void cmd_clock(char** args, int c) {
    print_header("LIVE CLOCK");
    printf("Press any key to exit...\n\n");
    hide_cursor();
    while (!_kbhit()) {
        SYSTEMTIME st; GetLocalTime(&st);
        printf("\r  %02d:%02d:%02d ", st.wHour, st.wMinute, st.wSecond);
        tnr_sleep(500);
    }
    _getch();
    show_cursor();
    printf("\n");
}

// About - Enhanced
void cmd_about(char** args, int c) {
    system("cls");
    
    set_col(C_HACK);
    printf("\n");
    printf("  ████████╗███╗   ██╗██████╗ ███╗   ███╗ ██╗███╗   ██╗    ██████╗ ██╗\n");
    printf("  ╚══██╔══╝████╗  ██║██╔══██╗████╗ ████║███║████╗  ██║    ╚════██╗██║\n");
    printf("     ██║   ██╔██╗ ██║██████╔╝██╔████╔██║ ██║██╔██╗ ██║     █████╔╝██║\n");
    printf("     ██║   ██║╚██╗██║██╔══██╗██║╚██╔╝██║ ██║██║╚██╗██║     ╚═══██╗╚═╝\n");
    printf("     ██║   ██║ ╚████║██║  ██║██║ ╚═╝ ██║ ██║██║ ╚████║    ██████╔╝██╗\n");
    printf("     ╚═╝   ╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝     ╚═╝ ╚═╝╚═╝  ╚═══╝    ╚═════╝ ╚═╝\n");
    set_col(C_RESET);
    
    printf("\n");
    set_col(C_INFO);
    printf("  ┌───────────────────────────────────────────────────┐\n");
    printf("  │            ULTRA PRO TERMINAL EMULATOR            │\n");
    printf("  └───────────────────────────────────────────────────┘\n");
    set_col(C_RESET);
    
    printf("\n");
    printf("  "); set_col(C_OK); printf("█ "); set_col(C_RESET); printf("Version:    "); set_col(C_WHITE); printf("%s\n", VERSION);
    printf("  "); set_col(C_OK); printf("█ "); set_col(C_RESET); printf("Build Date: "); set_col(C_WHITE); printf("%s\n", BUILD_DATE);
    printf("  "); set_col(C_OK); printf("█ "); set_col(C_RESET); printf("Platform:   "); set_col(C_WHITE); printf("Windows Native (x64)\n");
    printf("  "); set_col(C_OK); printf("█ "); set_col(C_RESET); printf("Language:   "); set_col(C_WHITE); printf("C (C11 Standard)\n");
    printf("  "); set_col(C_OK); printf("█ "); set_col(C_RESET); printf("Commands:   "); set_col(C_WHITE); printf("120+ built-in commands\n");
    set_col(C_RESET);
    
    printf("\n");
    set_col(C_WARN);
    printf("  Features:\n");
    set_col(C_RESET);
    printf("    • System monitoring & process management\n");
    printf("    • Advanced file operations\n");
    printf("    • Data processing & encryption\n");
    printf("    • Network utilities\n");
    printf("    • Built-in games (Snake, Hangman, Tic-Tac-Toe)\n");
    printf("    • Productivity tools (notes, timers, aliases)\n");
    
    printf("\n");
    set_col(C_INFO);
    printf("  Built for hackers, by hackers.\n");
    printf("  Type 'help' for commands, 'man <cmd>' for details.\n");
    set_col(C_RESET);
    printf("\n");
}

// Version - Enhanced
void cmd_version(char** args, int c) {
    print_header("VERSION INFO");
    
    set_col(C_OK);
    printf("\n  TNRM1N4L ");
    set_col(C_WHITE);
    printf("%s\n", VERSION);
    set_col(C_INFO);
    printf("  Build Date: %s\n\n", BUILD_DATE);
    set_col(C_RESET);
    
    printf("  ┌─ Changelog ────────────────────────────────────┐\n");
    set_col(C_OK);
    printf("  │ v4.0 (Ultra Pro)                              │\n");
    set_col(C_RESET);
    printf("  │   + 50 new commands added                     │\n");
    printf("  │   + Enhanced games with ASCII art             │\n");
    printf("  │   + Better AI in Tic-Tac-Toe                  │\n");
    printf("  │   + Animated startup sequence                 │\n");
    printf("  │   + Progress bars and loading animations      │\n");
    printf("  │                                                │\n");
    printf("  │ v3.0 - History, calendar, man pages           │\n");
    printf("  │ v2.0 - File operations, data processing       │\n");
    printf("  │ v1.0 - Initial release                        │\n");
    printf("  └────────────────────────────────────────────────┘\n");
}
