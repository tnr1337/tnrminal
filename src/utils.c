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

// Progress bar utility
void print_progress(int percent, int width) {
    int filled = (percent * width) / 100;
    printf("[");
    set_col(C_OK);
    for (int i = 0; i < filled; i++) printf("#");
    set_col(C_RESET);
    for (int i = filled; i < width; i++) printf("-");
    printf("] %d%%", percent);
}

// Loading animation
void print_loading(const char* msg, int ms) {
    const char* spinner = "|/-\\";
    hide_cursor();
    for (int i = 0; i < (ms / 100); i++) {
        printf("\r%s %c ", msg, spinner[i % 4]);
        tnr_sleep(100);
    }
    show_cursor();
    printf("\r%s Done!\n", msg);
}

// Box drawing
void print_box(const char* title, int width) {
    set_col(C_INFO);
    printf("+");
    for (int i = 0; i < width - 2; i++) printf("-");
    printf("+\n");
    
    int padding = (width - 2 - (int)strlen(title)) / 2;
    printf("|");
    for (int i = 0; i < padding; i++) printf(" ");
    set_col(C_OK);
    printf("%s", title);
    set_col(C_INFO);
    for (int i = 0; i < width - 2 - padding - (int)strlen(title); i++) printf(" ");
    printf("|\n");
    
    printf("+");
    for (int i = 0; i < width - 2; i++) printf("-");
    printf("+\n");
    set_col(C_RESET);
}
