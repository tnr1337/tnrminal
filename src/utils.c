#include "tnr.h"

// Globals
HANDLE hConsole;
int running = 1;
char current_dir[MAX_PATH];
char cmd_history[MAX_HISTORY][MAX_CMD_LEN];
int history_count = 0;
int history_index = 0;

void set_col(int c) {
    SetConsoleTextAttribute(hConsole, c);
}

void update_cwd() {
    _getcwd(current_dir, MAX_PATH);
}

void print_header(const char* title) {
    printf("\n");
    set_col(C_HEAD);
    printf("=== %s ===\n", title);
    set_col(C_RESET);
}

void print_error(const char* msg) {
    set_col(C_ERR);
    printf("Error: %s\n", msg);
    set_col(C_RESET);
}

void print_success(const char* msg) {
    set_col(C_OK);
    printf("%s\n", msg);
    set_col(C_RESET);
}

void print_usage(const char* cmd, const char* usage) {
    set_col(C_WARN);
    printf("Usage: %s %s\n", cmd, usage);
    set_col(C_RESET);
}

void tnr_sleep(int ms) {
    Sleep(ms);
}

void gotoxy(int x, int y) {
    COORD c = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hConsole, c);
}

void hide_cursor() {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &info);
}

void show_cursor() {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 20;
    info.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &info);
}

void add_to_history(const char* cmd) {
    if (cmd == NULL || strlen(cmd) == 0) return;
    
    // Don't add duplicates of last command
    if (history_count > 0 && strcmp(cmd_history[(history_count - 1) % MAX_HISTORY], cmd) == 0) {
        return;
    }
    
    safe_strcpy(cmd_history[history_count % MAX_HISTORY], cmd, MAX_CMD_LEN);
    history_count++;
    history_index = history_count;
}

int safe_strcpy(char* dest, const char* src, size_t dest_size) {
    if (dest == NULL || src == NULL || dest_size == 0) return 0;
    size_t src_len = strlen(src);
    size_t copy_len = (src_len < dest_size - 1) ? src_len : dest_size - 1;
    memcpy(dest, src, copy_len);
    dest[copy_len] = '\0';
    return (src_len < dest_size) ? 1 : 0;
}
