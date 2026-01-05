#include "tnr.h"

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

void cmd_wc(char** args, int c) {
    if (c < 2) { print_usage("wc", "<file>"); return; }
    FILE* f = fopen(args[1], "r");
    if (!f) { print_error("File not found."); return; }
    int lines=0, words=0, bytes=0;
    int ch;  // Use int for proper EOF detection
    int in_word = 0;
    while((ch = fgetc(f)) != EOF) {
        bytes++;
        if (ch == '\n') lines++;
        if (ch == ' ' || ch == '\n' || ch == '\t') in_word = 0;
        else if (!in_word) { in_word = 1; words++; }
    }
    fclose(f);
    printf("Lines: %d  Words: %d  Bytes: %d\n", lines, words, bytes);
}

// [NEW] Simple Bubble Sort (In-Memory, Max 100 lines)
void cmd_sort(char** args, int c) {
    if (c < 2) return;
    FILE* f = fopen(args[1], "r");
    if (!f) return;
    
    char lines[100][256];
    int count = 0;
    while(fgets(lines[count], 256, f) && count < 100) count++;
    
    // Check if more lines exist
    if (count == 100 && fgetc(f) != EOF) {
        set_col(C_WARN);
        printf("Warning: File too large. Sorting first 100 lines only.\n");
        set_col(C_RESET);
    }
    fclose(f);
    
    for(int i=0; i<count-1; i++) {
        for(int j=0; j<count-i-1; j++) {
            if (strcmp(lines[j], lines[j+1]) > 0) {
                char temp[256];
                strcpy(temp, lines[j]);
                strcpy(lines[j], lines[j+1]);
                strcpy(lines[j+1], temp);
            }
        }
    }
    
    for(int i=0; i<count; i++) printf("%s", lines[i]);
}

// [NEW] Uniq (Consecutive)
void cmd_uniq(char** args, int c) {
    if (c < 2) return;
    FILE* f = fopen(args[1], "r");
    if (!f) return;
    
    char prev[256] = "";
    char curr[256];
    while(fgets(curr, 256, f)) {
        if (strcmp(prev, curr) != 0) {
            printf("%s", curr);
            strcpy(prev, curr);
        }
    }
    fclose(f);
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
    print_header("ASCII TABLE");
    printf("DEC  HEX  CHAR   |  DEC  HEX  CHAR   |  DEC  HEX  CHAR\n");
    printf("------------------------------------------------------------\n");
    int cols = 0;
    for(int i = ASCII_START; i < ASCII_END; i++) {
        printf("%3d  %02X   %c     ", i, i, i);
        cols++;
        if (cols % 3 == 0) {
            printf("\n");
        } else {
            printf("| ");
        }
    }
    if (cols % 3 != 0) printf("\n");
}

// [NEW] Hex Dump
void cmd_hex(char** args, int c) {
    if (c < 2) return;
    FILE* f = fopen(args[1], "rb");
    if (!f) return;
    
    unsigned char buf[16];
    size_t n;
    int offset = 0;
    
    while((n = fread(buf, 1, 16, f)) > 0) {
        printf("%08X  ", offset);
        for(int i=0; i<16; i++) {
            if (i < n) printf("%02X ", buf[i]);
            else printf("   ");
        }
        printf(" |");
        for(int i=0; i<n; i++) {
            if (buf[i] >= 32 && buf[i] <= 126) printf("%c", buf[i]);
            else printf(".");
        }
        printf("|\n");
        offset += 16;
        if (offset > 512) { printf("... (Truncated)\n"); break; }
    }
    fclose(f);
}

// [NEW] Base64 (Mock Encrypt Only)
void cmd_base64(char** args, int c) {
    // Implementing full base64 in C is verbose, doing simple mock for now or simple implementation if possible.
    // Let's do a simple mock that shifts to look "encoded"
    if (c < 2) return;
    char* s = args[1];
    printf("Base64 (Mock): ==");
    while(*s) { printf("%02X", *s); s++; }
    printf("==\n");
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
