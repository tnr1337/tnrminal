#include "tnr.h"

// Globals
HANDLE hConsole;
int running = 1;
char current_dir[MAX_PATH];

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
