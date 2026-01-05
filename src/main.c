#include "tnr.h"

// Dispatch helper needed forward declaration for recursive calls (like script)
// command table needs to be visible or passed.
// Simplest: Define table here and helper here.

Command commands[] = {
    // System (15 + 10 new = 25)
    {"sys", cmd_sys, "System Monitor"},
    {"proc", cmd_proc, "Process List"},
    {"ps", cmd_ps, "Enhanced Process List"},
    {"fetch", cmd_fetch, "System Info Art"},
    {"whoami", cmd_whoami, "Current User"},
    {"hostname", cmd_hostname, "Hostname"},
    {"os", cmd_os, "OS Version"},
    {"uptime", cmd_uptime, "System Uptime"},
    {"drives", cmd_drives, "Logical Drives"},
    {"env", cmd_env, "Environment Variables"},
    {"time", cmd_time, "Show Time"},
    {"date", cmd_date, "Show Date"},
    {"shutdown", cmd_shutdown, "Simulate Shutdown"},
    {"reboot", cmd_reboot, "Simulate Reboot"},
    {"lock", cmd_lock, "Lock Session"},
    // NEW System
    {"kill", cmd_kill, "Kill Process"},
    {"taskmgr", cmd_taskmgr, "Task Manager"},
    {"meminfo", cmd_meminfo, "Memory Info"},
    {"cpuinfo", cmd_cpuinfo, "CPU Info"},
    {"battery", cmd_battery, "Battery Status"},
    {"diskinfo", cmd_diskinfo, "Disk Info"},
    {"services", cmd_services, "List Services"},
    {"netstat", cmd_netstat, "Network Stats"},
    {"datetime", cmd_datetime, "Full DateTime"},
    {"syslog", cmd_syslog, "System Log"},

    // File (18 + 10 new = 28)
    {"ls", cmd_ls, "List Directory"},
    {"dir", cmd_ls, "Alias for ls"},
    {"pwd", cmd_pwd, "Print Working Directory"},
    {"cd", cmd_cd, "Change Directory"},
    {"mkdir", cmd_mkdir, "Make Directory"},
    {"rmdir", cmd_rmdir, "Remove Directory"},
    {"mkfile", cmd_mkfile, "Create File"},
    {"touch", cmd_touch, "Update timestamp"},
    {"rm", cmd_rm, "Remove File"},
    {"cp", cmd_cp, "Copy File"},
    {"mv", cmd_mv, "Move File"},
    {"cat", cmd_cat, "Print Content"},
    {"type", cmd_cat, "Alias for cat"},
    {"head", cmd_head, "First 10 lines"},
    {"tail", cmd_tail, "Last 10 lines"},
    {"edit", cmd_edit, "Line Editor"},
    {"find", cmd_find, "Find File"},
    {"tree", cmd_tree, "Dir Tree"},
    {"diff", cmd_diff, "Compare Files"},
    {"du", cmd_du, "Disk Usage"},
    // NEW File
    {"rename", cmd_rename, "Rename File"},
    {"stat", cmd_stat, "File Statistics"},
    {"chmod", cmd_chmod, "File Permissions"},
    {"ln", cmd_ln, "Symlink Info"},
    {"truncate", cmd_truncate, "Truncate File"},
    {"append", cmd_append, "Append to File"},
    {"fsize", cmd_fsize, "File Size"},
    {"ftype", cmd_ftype, "File Type"},
    {"search", cmd_search, "Search Files"},
    {"zip", cmd_zip, "Archive Info"},

    // Data (13 + 8 new = 21)
    {"grep", cmd_grep, "Find text"},
    {"wc", cmd_wc, "Word Count"},
    {"sort", cmd_sort, "Sort Lines"},
    {"uniq", cmd_uniq, "Unique Lines"},
    {"upper", cmd_upper, "To Uppercase"},
    {"lower", cmd_lower, "To Lowercase"},
    {"rev", cmd_rev, "Reverse String"},
    {"bin", cmd_bin, "Dec to Binary"},
    {"ascii", cmd_ascii, "ASCII Table"},
    {"hex", cmd_hex, "Hex Dump"},
    {"base64", cmd_base64, "Base64 Encode"},
    {"encrypt", cmd_encrypt, "ROT13 Encrypt"},
    {"decrypt", cmd_decrypt, "ROT13 Decrypt"},
    // NEW Data
    {"md5", cmd_md5, "MD5 Hash"},
    {"sha256", cmd_sha256, "SHA256 Hash"},
    {"json", cmd_json, "JSON Viewer"},
    {"csv", cmd_csv, "CSV Viewer"},
    {"count", cmd_count, "Char Counter"},
    {"replace", cmd_replace, "Text Replace"},
    {"trim", cmd_trim, "Trim Text"},
    {"split", cmd_split, "Split String"},

    // Network (3 + 6 new = 9)
    {"ping", cmd_ping, "Ping Simulator"},
    {"ip", cmd_ip, "Show IP"},
    {"curl", cmd_curl, "HTTP Request Sim"},
    // NEW Network
    {"wget", cmd_wget, "Download Sim"},
    {"dns", cmd_dns, "DNS Lookup"},
    {"traceroute", cmd_traceroute, "Trace Route"},
    {"ifconfig", cmd_ifconfig, "Interface Config"},
    {"port", cmd_port, "Port Scanner"},
    {"http", cmd_http, "HTTP Tester"},

    // Fun & Games (9 + 8 new = 17)
    {"matrix", cmd_matrix, "Digital Rain"},
    {"snake", cmd_snake, "Snake Game"},
    {"weather", cmd_weather, "Sim Weather"},
    {"joke", cmd_joke, "Tell a joke"},
    {"fortune", cmd_fortune, "Random Quote"},
    {"rand", cmd_rand, "Random Number"},
    {"dice", cmd_dice, "Roll D6"},
    {"beep", cmd_beep, "System Beep"},
    {"selfdestruct", cmd_selfdestruct, "Don't run this"},
    // NEW Games
    {"hangman", cmd_hangman, "Hangman Game"},
    {"tictactoe", cmd_tictactoe, "Tic-Tac-Toe"},
    {"quiz", cmd_quiz, "Tech Quiz"},
    {"typing", cmd_typing, "Typing Test"},
    {"countdown", cmd_countdown, "Countdown"},
    {"stopwatch", cmd_stopwatch, "Stopwatch"},
    {"slots", cmd_slots, "Slot Machine"},
    {"rps", cmd_rps, "Rock-Paper-Scissors"},

    // Utils & Productivity (15 + 8 new = 23)
    {"calc", cmd_calc, "Basic Calculator"},
    {"todo", cmd_todo, "Todo Manager"},
    {"calendar", cmd_calendar, "Show Calendar"},
    {"pomodoro", cmd_pomodoro, "Focus Timer"},
    {"journal", cmd_journal, "Log Entry"},
    {"script", cmd_script, "Run Script"},
    {"history", cmd_history, "Command History"}, 
    {"clear", cmd_clear, "Clear Screen"},
    {"cls", cmd_clear, "Alias for clear"},
    {"echo", cmd_echo, "Print text"},
    {"color", cmd_color, "Set Text Color"},
    {"theme", cmd_theme, "Set Theme"},
    {"man", cmd_man, "Manual Page"},
    {"run", cmd_run, "Run System Cmd"},
    {"exit", cmd_exit, "Shutdown"},
    {"help", cmd_help, "List Commands"},
    // NEW Productivity
    {"alias", cmd_alias, "Cmd Aliases"},
    {"bookmark", cmd_bookmark, "Dir Bookmarks"},
    {"notes", cmd_notes, "Note Taking"},
    {"reminder", cmd_reminder, "Set Reminder"},
    {"timer", cmd_timer, "Timer"},
    {"clock", cmd_clock, "Live Clock"},
    {"about", cmd_about, "About Terminal"},
    {"version", cmd_version, "Version Info"}
};

void cmd_help(char** args, int c) {
    print_header("HELP MENU - 120+ COMMANDS");
    
    set_col(C_INFO);
    printf("\n  SYSTEM (25):\n");
    set_col(C_RESET);
    printf("    sys, proc, ps, fetch, whoami, hostname, os, uptime, drives, env\n");
    printf("    time, date, shutdown, reboot, lock, kill, taskmgr, meminfo\n");
    printf("    cpuinfo, battery, diskinfo, services, netstat, datetime, syslog\n");
    
    set_col(C_INFO);
    printf("\n  FILE (28):\n");
    set_col(C_RESET);
    printf("    ls, dir, pwd, cd, mkdir, rmdir, mkfile, touch, rm, cp, mv\n");
    printf("    cat, head, tail, edit, find, tree, diff, du, rename, stat\n");
    printf("    chmod, ln, truncate, append, fsize, ftype, search, zip\n");
    
    set_col(C_INFO);
    printf("\n  DATA (21):\n");
    set_col(C_RESET);
    printf("    grep, wc, sort, uniq, upper, lower, rev, bin, ascii, hex\n");
    printf("    base64, encrypt, decrypt, md5, sha256, json, csv, count\n");
    printf("    replace, trim, split\n");
    
    set_col(C_INFO);
    printf("\n  NETWORK (9):\n");
    set_col(C_RESET);
    printf("    ping, ip, curl, wget, dns, traceroute, ifconfig, port, http\n");
    
    set_col(C_INFO);
    printf("\n  FUN & GAMES (17):\n");
    set_col(C_RESET);
    printf("    matrix, snake, weather, joke, fortune, rand, dice, beep\n");
    printf("    hangman, tictactoe, quiz, typing, countdown, stopwatch, slots, rps\n");
    
    set_col(C_INFO);
    printf("\n  UTILITIES (23):\n");
    set_col(C_RESET);
    printf("    calc, todo, calendar, pomodoro, journal, script, history\n");
    printf("    clear, cls, echo, color, theme, man, run, exit, help\n");
    printf("    alias, bookmark, notes, reminder, timer, clock, about, version\n");
    
    set_col(C_WARN);
    printf("\n  TIP: Use 'man <command>' for detailed help. Use 'about' for info.\n");
    set_col(C_RESET);
}

void dispatch_command(char** args, int arg_c) {
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

int main() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    srand((unsigned int)time(NULL));
    update_cwd();

    // Startup Banner
    set_col(C_HACK);
    printf("\n");
    printf("  ████████╗███╗   ██╗██████╗ ███╗   ███╗██╗███╗   ██╗ █████╗ ██╗\n");
    printf("  ╚══██╔══╝████╗  ██║██╔══██╗████╗ ████║██║████╗  ██║██╔══██╗██║\n");
    printf("     ██║   ██╔██╗ ██║██████╔╝██╔████╔██║██║██╔██╗ ██║███████║██║\n");
    printf("     ██║   ██║╚██╗██║██╔══██╗██║╚██╔╝██║██║██║╚██╗██║██╔══██║██║\n");
    printf("     ██║   ██║ ╚████║██║  ██║██║ ╚═╝ ██║██║██║ ╚████║██║  ██║███████╗\n");
    printf("     ╚═╝   ╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚══════╝\n");
    set_col(C_RESET);
    printf("\n");
    set_col(C_OK);
    printf("  TNRM1N4L v%s | Custom Terminal Emulator\n", VERSION);
    set_col(C_INFO);
    printf("  Type 'help' for commands, 'man <cmd>' for detailed help.\n");
    set_col(C_RESET);

    char input[MAX_CMD_LEN];
    char input_copy[MAX_CMD_LEN];  // Keep original for history
    char* args[MAX_ARGS];

    while (running) {
        set_col(C_OK);
        printf("\n%s > ", current_dir);
        set_col(C_RESET);

        if (!fgets(input, MAX_CMD_LEN, stdin)) break;
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;

        // Save original input for history
        safe_strcpy(input_copy, input, MAX_CMD_LEN);

        // Tokenize
        int arg_c = 0;
        char* token = strtok(input, " ");
        while(token && arg_c < MAX_ARGS) {
            args[arg_c++] = token;
            token = strtok(NULL, " ");
        }

        if (arg_c == 0) continue;
        
        // Add to history before executing
        add_to_history(input_copy);
        
        dispatch_command(args, arg_c);
    }
    
    set_col(C_OK);
    printf("\nGoodbye! Terminal closed.\n");
    set_col(C_RESET);
    return 0;
}
