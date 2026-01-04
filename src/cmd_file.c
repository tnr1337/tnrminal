#include "tnr.h"

void cmd_ls(char** args, int c) {
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile("*", &fd);
    if (hFind == INVALID_HANDLE_VALUE) { printf("Empty or Error.\n"); return; }
    
    print_header("DIRECTORY LISTING");
    int count = 0;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            set_col(C_WARN);
            printf("<DIR>  %-20s", fd.cFileName);
        } else {
            set_col(C_RESET);
            printf("       %-20s", fd.cFileName);
        }
        count++;
        if (count % 3 == 0) printf("\n");
    } while(FindNextFile(hFind, &fd));
    if (count % 3 != 0) printf("\n");
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

void cmd_rmdir(char** args, int c) {
    if (c < 2) return;
    if (_rmdir(args[1]) == 0) printf("Removed.\n");
    else printf("Error.\n");
}

void cmd_mkfile(char** args, int c) {
    if (c < 2) return;
    FILE* f = fopen(args[1], "w");
    if (f) { fprintf(f, ""); fclose(f); printf("File touched.\n"); }
}

void cmd_rm(char** args, int c) {
    if (c < 2) return;
    if (DeleteFile(args[1])) printf("Deleted.\n");
    else printf("Error deleting file.\n");
}

void cmd_cp(char** args, int c) {
    if (c < 3) return;
    if (CopyFile(args[1], args[2], FALSE)) printf("Copied.\n");
    else printf("Copy failed.\n");
}

void cmd_mv(char** args, int c) {
    if (c < 3) return;
    if (MoveFile(args[1], args[2])) printf("Moved.\n");
    else printf("Move failed.\n");
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
    if (!f) return;
    // Count lines
    int lines = 0;
    char ch;
    while(!feof(f)) { if(fgetc(f) == '\n') lines++; }
    rewind(f);
    
    int current = 0;
    int start = lines - 10;
    if (start < 0) start = 0;
    
    char buf[1024];
    while(fgets(buf, sizeof(buf), f)) {
        if (current++ >= start) printf("%s", buf);
    }
    fclose(f);
}

void cmd_touch(char** args, int c) {
    if (c < 2) return;
    FILE* f = fopen(args[1], "a"); // Append mode touches without truncate
    if (f) { fclose(f); printf("File touched.\n"); }
    else { printf("Error accessing file.\n"); }
}

// [NEW] Simple Line Editor
void cmd_edit(char** args, int c) {
    if (c < 2) { printf("Usage: edit <filename>\n"); return; }
    char* filename = args[1];
    
    // Load file into memory (max 100 lines, 256 chars each mock)
    char lines[100][256];
    int line_count = 0;
    
    FILE* f = fopen(filename, "r");
    if (f) {
        while(fgets(lines[line_count], 256, f) && line_count < 100) {
            // Remove newline for editing
            lines[line_count][strcspn(lines[line_count], "\n")] = 0;
            line_count++;
        }
        fclose(f);
        printf("Loaded %d lines.\n", line_count);
    } else {
        printf("New file.\n");
    }

    // Editor Loop
    char input[256];
    int dirty = 0;
    while(1) {
        printf("\nEDIT Mode (list, add, del <n>, save, quit) > ");
        if (!fgets(input, 256, stdin)) break;
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "list") == 0) {
            for(int i=0; i<line_count; i++) printf("%3d: %s\n", i+1, lines[i]);
        } else if (strncmp(input, "add", 3) == 0) {
            if (line_count >= 100) { printf("Buffer full.\n"); continue; }
            printf("Enter line: ");
            fgets(lines[line_count], 256, stdin);
            lines[line_count][strcspn(lines[line_count], "\n")] = 0;
            line_count++;
            dirty = 1;
        } else if (strncmp(input, "del", 3) == 0) {
             if (strlen(input) <= 4) {
                 printf("Usage: del <line_number>\n");
                 continue;
             }
            int ln = atoi(input + 4);
            if (ln > 0 && ln <= line_count) {
                // Shift down
                for(int i=ln-1; i<line_count-1; i++) {
                    strcpy(lines[i], lines[i+1]);
                }
                line_count--;
                dirty = 1;
                printf("Line deleted.\n");
            } else {
                printf("Invalid line number.\n");
            }
        } else if (strcmp(input, "save") == 0) {
            f = fopen(filename, "w");
            if (f) {
                for(int i=0; i<line_count; i++) fprintf(f, "%s\n", lines[i]);
                fclose(f);
                printf("Saved.\n");
                dirty = 0;
            }
        } else if (strcmp(input, "quit") == 0) {
            if (dirty) printf("Warning: Unsaved changes. Type 'save' then 'quit'.\n");
            else break;
        }
    }
}

// [NEW] Recursive Find (Mock depth 1)
void cmd_find(char** args, int c) {
    if (c < 2) { printf("Usage: find <name>\n"); return; }
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile("*", &fd);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strstr(fd.cFileName, args[1])) {
                printf("Found: %s\n", fd.cFileName);
            }
        } while(FindNextFile(hFind, &fd));
        FindClose(hFind);
    }
}

void cmd_tree(char** args, int c) {
    printf(".\n");
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile("*", &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (fd.cFileName[0] != '.') printf("+--- %s\n", fd.cFileName);
            } else {
                printf("|    %s\n", fd.cFileName);
            }
        } while(FindNextFile(hFind, &fd));
        FindClose(hFind);
    }
}

// [NEW] Diff
void cmd_diff(char** args, int c) {
    if (c < 3) { printf("Usage: diff <f1> <f2>\n"); return; }
    // Simple mock diff: compare size and first line
    FILE *f1 = fopen(args[1], "r");
    FILE *f2 = fopen(args[2], "r");
    if (!f1 || !f2) { printf("Error opening files.\n"); return; }
    
    char b1[256], b2[256];
    int diffs = 0;
    int line = 1;
    
    char* p1 = fgets(b1, 256, f1);
    char* p2 = fgets(b2, 256, f2);

    while(p1 || p2) {
        // Remove newlines for cleaner output display if desired, 
        // but keeping them for now to match original style roughly.
        if (p1) b1[strcspn(b1, "\n")] = 0;
        if (p2) b2[strcspn(b2, "\n")] = 0;

        if ((p1 && !p2) || (!p1 && p2) || (p1 && p2 && strcmp(b1, b2) != 0)) {
            printf("Line %d:\n", line);
            printf("< %s\n", p1 ? b1 : "(EOF)");
            printf("> %s\n", p2 ? b2 : "(EOF)");
            diffs++;
        }
        
        p1 = fgets(b1, 256, f1);
        p2 = fgets(b2, 256, f2);
        line++;
    }

    if (diffs == 0) printf("Files are identical.\n");
    fclose(f1); fclose(f2);
}

// [NEW] DU
void cmd_du(char** args, int c) {
    // List file sizes in current dir
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile("*", &fd);
    unsigned long long total = 0;
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                ULARGE_INTEGER filesize;
                filesize.LowPart = fd.nFileSizeLow;
                filesize.HighPart = fd.nFileSizeHigh;
                total += filesize.QuadPart;
                printf("%-20s %llu bytes\n", fd.cFileName, filesize.QuadPart);
            }
        } while(FindNextFile(hFind, &fd));
        FindClose(hFind);
        printf("Total: %llu bytes\n", total);
    }
}
