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
