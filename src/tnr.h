#ifndef TNR_H
#define TNR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <direct.h> // _getcwd, _chdir
#include <conio.h> // _getch
#include <sys/stat.h>

// --- Version & Identity ---
#define VERSION "4.0 (Ultra Pro)"
#define HOST_NAME "TNRM1N4L"
#define BUILD_DATE __DATE__

// --- Buffer Sizes ---
#define MAX_CMD_LEN 1024
#define MAX_ARGS 64
#define MAX_LINE_LEN 256
#define MAX_LINES_BUFFER 100
#define MAX_HISTORY 100
#define MAX_USERNAME 256
#define MAX_HOSTNAME 256
#define MAX_ALIASES 50
#define MAX_BOOKMARKS 20
#define MAX_NOTES 50

// --- Display Constants ---
#define COLUMNS_HELP 3
#define COLUMNS_LS 3
#define HEAD_TAIL_LINES 10
#define ASCII_START 32
#define ASCII_END 127
#define ASCII_COLS 8
#define HEX_BYTES_PER_LINE 16
#define HEX_TRUNCATE_OFFSET 512
#define PROC_DISPLAY_LIMIT 40

// --- Colors ---
#define C_RESET 7
#define C_OK    10 // Green
#define C_ERR   12 // Red
#define C_INFO  11 // Cyan
#define C_WARN  14 // Yellow
#define C_HEAD  13 // Magenta
#define C_HACK  2  // Dark Green (Matrix)
#define C_TITLE 9  // Blue
#define C_GOLD  6  // Gold/Brown
#define C_WHITE 15 // Bright White

// --- Structs ---
typedef struct {
    char* name;
    void (*func)(char**, int);
    char* desc;
} Command;

// --- Globals (External) ---
extern HANDLE hConsole;
extern int running;
extern char current_dir[MAX_PATH];
extern char cmd_history[MAX_HISTORY][MAX_CMD_LEN];
extern int history_count;
extern int history_index;

// --- Prototypes ---

// Utils
void set_col(int c);
void update_cwd();
void print_header(const char* title);
void print_error(const char* msg);
void print_success(const char* msg);
void print_usage(const char* cmd, const char* usage);
void tnr_sleep(int ms);
void gotoxy(int x, int y);
void hide_cursor();
void show_cursor();
void print_progress(int percent, int width);
void print_loading(const char* msg, int ms);
void print_box(const char* title, int width);
void add_to_history(const char* cmd);
int safe_strcpy(char* dest, const char* src, size_t dest_size);

// System
void cmd_sys(char** args, int c);
void cmd_proc(char** args, int c);
void cmd_whoami(char** args, int c);
void cmd_hostname(char** args, int c);
void cmd_os(char** args, int c);
void cmd_uptime(char** args, int c);
void cmd_drives(char** args, int c);
void cmd_env(char** args, int c);
void cmd_time(char** args, int c);
void cmd_date(char** args, int c);
void cmd_fetch(char** args, int c);
void cmd_ps(char** args, int c);
void cmd_shutdown(char** args, int c);
void cmd_reboot(char** args, int c);
void cmd_lock(char** args, int c);
// New System Commands
void cmd_kill(char** args, int c);
void cmd_taskmgr(char** args, int c);
void cmd_meminfo(char** args, int c);
void cmd_cpuinfo(char** args, int c);
void cmd_battery(char** args, int c);
void cmd_diskinfo(char** args, int c);
void cmd_services(char** args, int c);
void cmd_netstat(char** args, int c);
void cmd_datetime(char** args, int c);
void cmd_syslog(char** args, int c);

// File
void cmd_ls(char** args, int c);
void cmd_pwd(char** args, int c);
void cmd_cd(char** args, int c);
void cmd_mkdir(char** args, int c);
void cmd_rmdir(char** args, int c);
void cmd_mkfile(char** args, int c);
void cmd_rm(char** args, int c);
void cmd_cp(char** args, int c);
void cmd_mv(char** args, int c);
void cmd_cat(char** args, int c);
void cmd_head(char** args, int c);
void cmd_tail(char** args, int c);
void cmd_touch(char** args, int c);
void cmd_edit(char** args, int c);
void cmd_find(char** args, int c);
void cmd_tree(char** args, int c);
void cmd_diff(char** args, int c);
void cmd_du(char** args, int c);
// New File Commands
void cmd_rename(char** args, int c);
void cmd_stat(char** args, int c);
void cmd_chmod(char** args, int c);
void cmd_ln(char** args, int c);
void cmd_truncate(char** args, int c);
void cmd_append(char** args, int c);
void cmd_fsize(char** args, int c);
void cmd_ftype(char** args, int c);
void cmd_search(char** args, int c);
void cmd_zip(char** args, int c);

// Data
void cmd_grep(char** args, int c);
void cmd_wc(char** args, int c);
void cmd_sort(char** args, int c);
void cmd_uniq(char** args, int c);
void cmd_upper(char** args, int c);
void cmd_lower(char** args, int c);
void cmd_rev(char** args, int c);
void cmd_bin(char** args, int c);
void cmd_ascii(char** args, int c);
void cmd_hex(char** args, int c);
void cmd_base64(char** args, int c);
void cmd_encrypt(char** args, int c);
void cmd_decrypt(char** args, int c);
// New Data Commands
void cmd_md5(char** args, int c);
void cmd_sha256(char** args, int c);
void cmd_json(char** args, int c);
void cmd_csv(char** args, int c);
void cmd_count(char** args, int c);
void cmd_replace(char** args, int c);
void cmd_trim(char** args, int c);
void cmd_split(char** args, int c);

// Network
void cmd_ping(char** args, int c);
void cmd_ip(char** args, int c);
void cmd_curl(char** args, int c);
// New Network Commands
void cmd_wget(char** args, int c);
void cmd_dns(char** args, int c);
void cmd_traceroute(char** args, int c);
void cmd_ifconfig(char** args, int c);
void cmd_port(char** args, int c);
void cmd_http(char** args, int c);

// Fun
void cmd_matrix(char** args, int c);
void cmd_snake(char** args, int c);
void cmd_weather(char** args, int c);
void cmd_joke(char** args, int c);
void cmd_fortune(char** args, int c);
void cmd_beep(char** args, int c);
void cmd_selfdestruct(char** args, int c);
void cmd_rand(char** args, int c);
void cmd_dice(char** args, int c);
// New Games Commands
void cmd_hangman(char** args, int c);
void cmd_tictactoe(char** args, int c);
void cmd_quiz(char** args, int c);
void cmd_typing(char** args, int c);
void cmd_countdown(char** args, int c);
void cmd_stopwatch(char** args, int c);
void cmd_slots(char** args, int c);
void cmd_rps(char** args, int c);

// Extra/Utils
void cmd_calc(char** args, int c);
void cmd_todo(char** args, int c);
void cmd_calendar(char** args, int c);
void cmd_pomodoro(char** args, int c);
void cmd_journal(char** args, int c);
void cmd_script(char** args, int c);
void cmd_history(char** args, int c);
void cmd_clear(char** args, int c);
void cmd_echo(char** args, int c);
void cmd_color(char** args, int c);
void cmd_theme(char** args, int c);
void cmd_help(char** args, int c);
void cmd_man(char** args, int c);
void cmd_run(char** args, int c);
void cmd_exit(char** args, int c);
// New Productivity Commands
void cmd_alias(char** args, int c);
void cmd_bookmark(char** args, int c);
void cmd_notes(char** args, int c);
void cmd_reminder(char** args, int c);
void cmd_timer(char** args, int c);
void cmd_clock(char** args, int c);
void cmd_about(char** args, int c);
void cmd_version(char** args, int c);

// Shared Dispatch Helper
void dispatch_command(char** args, int arg_c);

#endif
