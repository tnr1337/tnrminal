#include "tnr.h"

// Dispatch helper needed forward declaration for recursive calls (like script)
// command table needs to be visible or passed.
// Simplest: Define table here and helper here.

Command commands[] = {
    // System
    {"sys", cmd_sys, "System Monitor"},
    {"proc", cmd_proc, "Process List"},
    {"ps", cmd_ps, "Enhanced Process List"}, // NEW
    {"fetch", cmd_fetch, "System Info Art"}, // NEW
    {"whoami", cmd_whoami, "Current User"},
    {"hostname", cmd_hostname, "Hostname"},
    {"os", cmd_os, "OS Version"},
    {"uptime", cmd_uptime, "System Uptime"},
    {"drives", cmd_drives, "Logical Drives"},
    {"env", cmd_env, "Environment Variables"},
    {"time", cmd_time, "Show Time"},
    {"date", cmd_date, "Show Date"},
    {"shutdown", cmd_shutdown, "Simulate Shutdown"}, // NEW
    {"reboot", cmd_reboot, "Simulate Reboot"}, // NEW
    {"lock", cmd_lock, "Lock Session"}, // NEW

    // File
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
    {"edit", cmd_edit, "Line Editor"}, // NEW
    {"find", cmd_find, "Find File"}, // NEW
    {"tree", cmd_tree, "Dir Tree"},
    {"diff", cmd_diff, "Compare Files"}, // NEW
    {"du", cmd_du, "Disk Usage"}, // NEW

    // Data
    {"grep", cmd_grep, "Find text"},
    {"wc", cmd_wc, "Word Count"},
    {"sort", cmd_sort, "Sort Lines"}, // NEW
    {"uniq", cmd_uniq, "Unique Lines"}, // NEW
    {"upper", cmd_upper, "To Uppercase"},
    {"rev", cmd_rev, "Reverse String"},
    {"bin", cmd_bin, "Dec to Binary"},
    {"ascii", cmd_ascii, "ASCII Table"},
    {"hex", cmd_hex, "Hex Dump"}, // NEW
    {"base64", cmd_base64, "Base64 Encode"}, // NEW
    {"encrypt", cmd_encrypt, "Simple Encrypt"},
    {"decrypt", cmd_decrypt, "Simple Decrypt"},

    // Network
    {"ping", cmd_ping, "Ping Simulator"}, // NEW
    {"ip", cmd_ip, "Show IP"}, // NEW
    {"curl", cmd_curl, "HTTP Request Sim"}, // NEW

    // Fun
    {"matrix", cmd_matrix, "Digital Rain"}, // NEW
    {"snake", cmd_snake, "Snake Game"}, // NEW
    {"weather", cmd_weather, "Sim Weather"},
    {"joke", cmd_joke, "Tell a joke"}, // NEW
    {"fortune", cmd_fortune, "Random Quote"}, // NEW
    {"rand", cmd_rand, "Random Number"},
    {"dice", cmd_dice, "Roll D6"},
    {"beep", cmd_beep, "System Beep"},
    {"selfdestruct", cmd_selfdestruct, "Don't run this"},

    // Utils & Productivity
    {"calc", cmd_calc, "Basic Calculator"},
    {"todo", cmd_todo, "Todo Manager"}, // NEW
    {"calendar", cmd_calendar, "Show Calendar"}, // NEW
    {"pomodoro", cmd_pomodoro, "Focus Timer"}, // NEW
    {"journal", cmd_journal, "Log Entry"}, // NEW
    {"script", cmd_script, "Run Script"}, // NEW
    {"history", cmd_history, "Command History"}, 
    {"clear", cmd_clear, "Clear Screen"},
    {"cls", cmd_clear, "Alias for clear"},
    {"echo", cmd_echo, "Print text"},
    {"color", cmd_color, "Set Text Color"},
    {"theme", cmd_theme, "Set Theme"}, // NEW
    {"man", cmd_man, "Manual Page"}, // NEW
    {"run", cmd_run, "Run System Cmd"},
    {"exit", cmd_exit, "Shutdown"},
    {"help", cmd_help, "List Commands"}
};

void cmd_help(char** args, int c) {
    print_header("HELP MENU");
    
    set_col(C_INFO);
    printf("\n  SYSTEM COMMANDS:\n");
    set_col(C_RESET);
    printf("    sys, proc, ps, fetch, whoami, hostname, os, uptime, drives, env\n");
    printf("    time, date, shutdown, reboot, lock\n");
    
    set_col(C_INFO);
    printf("\n  FILE COMMANDS:\n");
    set_col(C_RESET);
    printf("    ls, dir, pwd, cd, mkdir, rmdir, mkfile, touch, rm, cp, mv\n");
    printf("    cat, head, tail, edit, find, tree, diff, du\n");
    
    set_col(C_INFO);
    printf("\n  DATA COMMANDS:\n");
    set_col(C_RESET);
    printf("    grep, wc, sort, uniq, upper, rev, bin, ascii, hex\n");
    printf("    base64, encrypt, decrypt\n");
    
    set_col(C_INFO);
    printf("\n  NETWORK COMMANDS:\n");
    set_col(C_RESET);
    printf("    ping, ip, curl\n");
    
    set_col(C_INFO);
    printf("\n  FUN & GAMES:\n");
    set_col(C_RESET);
    printf("    matrix, snake, weather, joke, fortune, rand, dice, beep\n");
    
    set_col(C_INFO);
    printf("\n  UTILITIES:\n");
    set_col(C_RESET);
    printf("    calc, todo, calendar, pomodoro, journal, script, history\n");
    printf("    clear, cls, echo, color, theme, man, run, exit\n");
    
    set_col(C_WARN);
    printf("\n  TIP: Use 'man <command>' for detailed help on any command.\n");
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
