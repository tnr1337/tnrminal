#include "tnr.h"

void cmd_grep(char** args, int c) {
    if (c < 3) { print_usage("grep", "<pattern> <file>"); return; }
    FILE* f = fopen(args[2], "r");
    if (!f) { print_error("File not found."); return; }
    
    char buf[1024];
    int line = 1;
    int matches = 0;
    
    while(fgets(buf, sizeof(buf), f)) {
        if (strstr(buf, args[1])) {
            set_col(C_INFO);
            printf("%d: ", line);
            set_col(C_RESET);
            
            // Highlight the match
            char* pos = buf;
            char* match;
            while ((match = strstr(pos, args[1])) != NULL) {
                // Print text before match
                while (pos < match) {
                    putchar(*pos++);
                }
                // Print match in color
                set_col(C_OK);
                printf("%s", args[1]);
                set_col(C_RESET);
                pos = match + strlen(args[1]);
            }
            // Print remaining text
            printf("%s", pos);
            matches++;
        }
        line++;
    }
    fclose(f);
    
    set_col(C_INFO);
    printf("\nFound %d matching line(s).\n", matches);
    set_col(C_RESET);
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
    if (c < 2) { print_usage("sort", "<file>"); return; }
    FILE* f = fopen(args[1], "r");
    if (!f) { print_error("File not found."); return; }
    
    char lines[MAX_LINES_BUFFER][MAX_LINE_LEN];
    int count = 0;
    while(fgets(lines[count], MAX_LINE_LEN, f) && count < MAX_LINES_BUFFER) count++;
    
    // Check if more lines exist
    if (count == MAX_LINES_BUFFER && fgetc(f) != EOF) {
        set_col(C_WARN);
        printf("Warning: File too large. Sorting first %d lines only.\n", MAX_LINES_BUFFER);
        set_col(C_RESET);
    }
    fclose(f);
    
    // Bubble sort
    for(int i=0; i<count-1; i++) {
        for(int j=0; j<count-i-1; j++) {
            if (strcmp(lines[j], lines[j+1]) > 0) {
                char temp[MAX_LINE_LEN];
                strcpy(temp, lines[j]);
                strcpy(lines[j], lines[j+1]);
                strcpy(lines[j+1], temp);
            }
        }
    }
    
    print_header("SORTED OUTPUT");
    for(int i=0; i<count; i++) printf("%s", lines[i]);
}

// [NEW] Uniq (Consecutive)
void cmd_uniq(char** args, int c) {
    if (c < 2) { print_usage("uniq", "<file>"); return; }
    FILE* f = fopen(args[1], "r");
    if (!f) { print_error("File not found."); return; }
    
    print_header("UNIQUE LINES");
    char prev[MAX_LINE_LEN] = "";
    char curr[MAX_LINE_LEN];
    int duplicates = 0;
    
    while(fgets(curr, MAX_LINE_LEN, f)) {
        if (strcmp(prev, curr) != 0) {
            printf("%s", curr);
            strcpy(prev, curr);
        } else {
            duplicates++;
        }
    }
    fclose(f);
    
    set_col(C_INFO);
    printf("\n(%d duplicate line(s) removed)\n", duplicates);
    set_col(C_RESET);
}

void cmd_upper(char** args, int c) {
    if (c < 2) { print_usage("upper", "<text>"); return; }
    set_col(C_OK);
    char* s = args[1];
    while(*s) {
        if (*s >= 'a' && *s <= 'z') printf("%c", *s - 32);
        else printf("%c", *s);
        s++;
    }
    set_col(C_RESET);
    printf("\n");
}

void cmd_lower(char** args, int c) {
    if (c < 2) { print_usage("lower", "<text>"); return; }
    set_col(C_OK);
    char* s = args[1];
    while(*s) {
        if (*s >= 'A' && *s <= 'Z') printf("%c", *s + 32);
        else printf("%c", *s);
        s++;
    }
    set_col(C_RESET);
    printf("\n");
}

void cmd_rev(char** args, int c) {
    if (c < 2) { print_usage("rev", "<text>"); return; }
    set_col(C_OK);
    for(int i=(int)strlen(args[1])-1; i>=0; i--) printf("%c", args[1][i]);
    set_col(C_RESET);
    printf("\n");
}

void cmd_bin(char** args, int c) {
    if (c < 2) { print_usage("bin", "<decimal_number>"); return; }
    int num = atoi(args[1]);
    printf("Decimal: %d\nBinary:  ", num);
    set_col(C_OK);
    
    // Skip leading zeros
    int started = 0;
    for(int i=31; i>=0; i--) {
        int bit = (num >> i) & 1;
        if (bit) started = 1;
        if (started || i == 0) {
            printf("%d", bit);
            if (i % 4 == 0 && i > 0) printf(" ");
        }
    }
    set_col(C_RESET);
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

// [NEW] Hex Dump - Improved
void cmd_hex(char** args, int c) {
    if (c < 2) { print_usage("hex", "<file>"); return; }
    FILE* f = fopen(args[1], "rb");
    if (!f) { print_error("File not found."); return; }
    
    print_header("HEX DUMP");
    printf("Offset    00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  ASCII\n");
    printf("--------  -----------------------------------------------  ----------------\n");
    
    unsigned char buf[HEX_BYTES_PER_LINE];
    size_t n;
    int offset = 0;
    
    while((n = fread(buf, 1, HEX_BYTES_PER_LINE, f)) > 0) {
        set_col(C_INFO);
        printf("%08X  ", offset);
        set_col(C_RESET);
        
        // Hex values
        for(size_t i = 0; i < HEX_BYTES_PER_LINE; i++) {
            if (i < n) {
                if (buf[i] == 0x00) {
                    set_col(C_WARN);
                } else if (buf[i] >= 32 && buf[i] <= 126) {
                    set_col(C_OK);
                } else {
                    set_col(C_RESET);
                }
                printf("%02X ", buf[i]);
            } else {
                printf("   ");
            }
            if (i == 7) printf(" ");
        }
        
        set_col(C_RESET);
        printf(" ");
        
        // ASCII representation
        for(size_t i = 0; i < n; i++) {
            if (buf[i] >= 32 && buf[i] <= 126) {
                set_col(C_OK);
                printf("%c", buf[i]);
            } else {
                set_col(C_WARN);
                printf(".");
            }
        }
        set_col(C_RESET);
        printf("\n");
        
        offset += HEX_BYTES_PER_LINE;
        if (offset > HEX_TRUNCATE_OFFSET) {
            set_col(C_WARN);
            printf("... (Truncated at %d bytes)\n", HEX_TRUNCATE_OFFSET);
            set_col(C_RESET);
            break;
        }
    }
    fclose(f);
}

// Real Base64 Encoding
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void cmd_base64(char** args, int c) {
    if (c < 2) { print_usage("base64", "<text>"); return; }
    
    const unsigned char* src = (const unsigned char*)args[1];
    size_t len = strlen(args[1]);
    
    printf("Base64: ");
    set_col(C_OK);
    
    size_t i;
    for (i = 0; i + 2 < len; i += 3) {
        putchar(base64_table[src[i] >> 2]);
        putchar(base64_table[((src[i] & 0x03) << 4) | (src[i + 1] >> 4)]);
        putchar(base64_table[((src[i + 1] & 0x0F) << 2) | (src[i + 2] >> 6)]);
        putchar(base64_table[src[i + 2] & 0x3F]);
    }
    
    if (i < len) {
        putchar(base64_table[src[i] >> 2]);
        if (i + 1 < len) {
            putchar(base64_table[((src[i] & 0x03) << 4) | (src[i + 1] >> 4)]);
            putchar(base64_table[(src[i + 1] & 0x0F) << 2]);
        } else {
            putchar(base64_table[(src[i] & 0x03) << 4]);
            putchar('=');
        }
        putchar('=');
    }
    
    set_col(C_RESET);
    printf("\n");
}

// ROT13 Encryption (Classic, reversible)
void cmd_encrypt(char** args, int c) {
    if (c < 2) { print_usage("encrypt", "<text>"); return; }
    
    printf("Encrypted: ");
    set_col(C_WARN);
    
    char* s = args[1];
    while(*s) {
        char ch = *s;
        if ((ch >= 'a' && ch <= 'z')) {
            ch = ((ch - 'a' + 13) % 26) + 'a';
        } else if ((ch >= 'A' && ch <= 'Z')) {
            ch = ((ch - 'A' + 13) % 26) + 'A';
        }
        putchar(ch);
        s++;
    }
    
    set_col(C_RESET);
    printf("\n");
}

void cmd_decrypt(char** args, int c) {
    // ROT13 is symmetric - same as encrypt
    if (c < 2) { print_usage("decrypt", "<text>"); return; }
    
    printf("Decrypted: ");
    set_col(C_OK);
    
    char* s = args[1];
    while(*s) {
        char ch = *s;
        if ((ch >= 'a' && ch <= 'z')) {
            ch = ((ch - 'a' + 13) % 26) + 'a';
        } else if ((ch >= 'A' && ch <= 'Z')) {
            ch = ((ch - 'A' + 13) % 26) + 'A';
        }
        putchar(ch);
        s++;
    }
    
    set_col(C_RESET);
    printf("\n");
}
