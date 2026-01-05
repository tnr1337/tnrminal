#include "tnr.h"

// Command descriptions for man pages
typedef struct {
    const char* name;
    const char* usage;
    const char* desc;
} ManPage;

static ManPage man_pages[] = {
    // System Commands
    {"sys", "", "Display system memory information including RAM usage and page file."},
    {"proc", "", "List all running processes with their PIDs."},
    {"ps", "", "Enhanced process list (alias for proc)."},
    {"fetch", "", "Display system info with ASCII art banner."},
    {"whoami", "", "Display the current logged-in username."},
    {"hostname", "", "Display the computer's hostname."},
    {"os", "", "Display the operating system version."},
    {"uptime", "", "Show how long the system has been running."},
    {"drives", "", "List all logical drives in the system."},
    {"env", "", "Display all environment variables."},
    {"time", "", "Show the current system time."},
    {"date", "", "Show the current date."},
    {"shutdown", "", "Simulate system shutdown (exits terminal)."},
    {"reboot", "", "Simulate system reboot (clears screen)."},
    {"lock", "", "Lock the terminal session (any password unlocks)."},
    
    // File Commands
    {"ls", "[path]", "List files and directories in current directory."},
    {"dir", "[path]", "Alias for ls command."},
    {"pwd", "", "Print the current working directory."},
    {"cd", "<path>", "Change the current directory."},
    {"mkdir", "<name>", "Create a new directory."},
    {"rmdir", "<name>", "Remove an empty directory."},
    {"mkfile", "<name>", "Create an empty file."},
    {"touch", "<name>", "Update file timestamp or create if not exists."},
    {"rm", "<file>", "Delete a file."},
    {"cp", "<src> <dest>", "Copy a file from source to destination."},
    {"mv", "<src> <dest>", "Move or rename a file."},
    {"cat", "<file>", "Display the contents of a file."},
    {"head", "<file>", "Display the first 10 lines of a file."},
    {"tail", "<file>", "Display the last 10 lines of a file."},
    {"edit", "<file>", "Open simple line editor for a file."},
    {"find", "<name>", "Search for files matching the pattern."},
    {"tree", "", "Display directory structure as a tree."},
    {"diff", "<f1> <f2>", "Compare two files line by line."},
    {"du", "", "Display disk usage for files in current directory."},
    
    // Data Commands
    {"grep", "<pattern> <file>", "Search for a pattern in a file."},
    {"wc", "<file>", "Count lines, words, and bytes in a file."},
    {"sort", "<file>", "Sort lines in a file alphabetically."},
    {"uniq", "<file>", "Filter out consecutive duplicate lines."},
    {"upper", "<text>", "Convert text to uppercase."},
    {"rev", "<text>", "Reverse a string."},
    {"bin", "<number>", "Convert decimal to binary."},
    {"ascii", "", "Display the ASCII table."},
    {"hex", "<file>", "Display hexadecimal dump of a file."},
    {"base64", "<text>", "Encode text to base64 (mock)."},
    {"encrypt", "<text>", "Simple ROT1 encryption."},
    {"decrypt", "<text>", "Simple ROT1 decryption."},
    
    // Network Commands
    {"ping", "<host>", "Simulate ping to a host."},
    {"ip", "", "Display network adapter information."},
    {"curl", "<url>", "Simulate HTTP request to a URL."},
    
    // Fun Commands
    {"matrix", "", "Display Matrix-style digital rain effect."},
    {"snake", "", "Play the classic Snake game."},
    {"weather", "", "Display simulated weather information."},
    {"joke", "", "Tell a random programming joke."},
    {"fortune", "", "Display a random inspirational quote."},
    {"rand", "", "Generate a random number."},
    {"dice", "", "Roll a 6-sided die."},
    {"beep", "", "Make a system beep sound."},
    {"selfdestruct", "", "Don't run this! (just kidding)"},
    
    // Utils Commands
    {"calc", "<n1> <op> <n2>", "Basic calculator (+, -, *, /, %%, ^)."},
    {"todo", "[add <task>|clear]", "Manage a simple todo list."},
    {"calendar", "[month] [year]", "Display a calendar for the specified month."},
    {"pomodoro", "", "Start a 25-minute focus timer."},
    {"journal", "<entry>", "Add a timestamped journal entry."},
    {"script", "<file.tnr>", "Execute commands from a script file."},
    {"history", "", "Display command history."},
    {"clear", "", "Clear the screen."},
    {"cls", "", "Alias for clear."},
    {"echo", "<text>", "Print text to the screen."},
    {"color", "<code>", "Set text color (0-15)."},
    {"theme", "<name>", "Set color theme (hack, retro, ocean)."},
    {"man", "<command>", "Display manual page for a command."},
    {"run", "<cmd>", "Execute a system command."},
    {"exit", "", "Exit the terminal."},
    {"help", "", "List all available commands."},
    {NULL, NULL, NULL}
};

// [NEW] Todo List
void cmd_todo(char** args, int c) {
    if (c < 2) {
        // List
        FILE* f = fopen("todo.txt", "r");
        if (!f) { printf("No todos.\n"); return; }
        char buf[256];
        int i=1;
        while(fgets(buf, 256, f)) printf("%d. %s", i++, buf);
        fclose(f);
        return;
    }
    
    // Add
    if (strcmp(args[1], "add") == 0 && c >= 3) {
        FILE* f = fopen("todo.txt", "a");
        for(int i=2; i<c; i++) fprintf(f, "%s ", args[i]);
        fprintf(f, "\n");
        fclose(f);
        printf("Added.\n");
    } 
    // Clear
    else if (strcmp(args[1], "clear") == 0) {
        unlink("todo.txt");
        printf("Cleared.\n");
    }
}

// [NEW] Script Runner
void cmd_script(char** args, int c) {
    if (c < 2) { printf("Usage: script <file.tnr>\n"); return; }
    FILE* f = fopen(args[1], "r");
    if (!f) { printf("Script not found.\n"); return; }
    
    char line[MAX_CMD_LEN];
    char* s_args[MAX_ARGS];
    
    while(fgets(line, MAX_CMD_LEN, f)) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0 || line[0] == '#') continue; 
        
        printf("EXEC: %s\n", line); // Echo execution
        
        // Quick Tokenize (Dup logic, ideally helper function)
        int ac = 0;
        char* token = strtok(line, " ");
        while(token && ac < MAX_ARGS) {
            s_args[ac++] = token;
            token = strtok(NULL, " ");
        }
        if (ac > 0) dispatch_command(s_args, ac);
    }
    fclose(f);
}

// [NEW] Calendar - Dynamic
static int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int days_in_month(int month, int year) {
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && is_leap_year(year)) return 29;
    return days[month - 1];
}

static int day_of_week(int day, int month, int year) {
    // Zeller's congruence for Gregorian calendar
    if (month < 3) { month += 12; year--; }
    int k = year % 100;
    int j = year / 100;
    int h = (day + (13 * (month + 1)) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
    return ((h + 6) % 7);  // Convert to 0=Sun, 1=Mon, ..., 6=Sat then adjust to Monday first
}

void cmd_calendar(char** args, int c) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    int month = st.wMonth;
    int year = st.wYear;
    
    // Parse optional arguments
    if (c >= 2) month = atoi(args[1]);
    if (c >= 3) year = atoi(args[2]);
    
    if (month < 1 || month > 12) { print_error("Invalid month (1-12)."); return; }
    if (year < 1) { print_error("Invalid year."); return; }
    
    const char* months[] = {"January", "February", "March", "April", "May", "June",
                           "July", "August", "September", "October", "November", "December"};
    
    print_header("CALENDAR");
    set_col(C_INFO);
    printf("    %s %d\n", months[month - 1], year);
    set_col(C_RESET);
    printf("Mo Tu We Th Fr Sa Su\n");
    
    int first_day = day_of_week(1, month, year);
    int total_days = days_in_month(month, year);
    
    // Adjust first_day (0=Sun in Zeller, we want 0=Mon)
    first_day = (first_day == 0) ? 6 : first_day - 1;
    
    // Print leading spaces
    for (int i = 0; i < first_day; i++) printf("   ");
    
    // Print days
    for (int day = 1; day <= total_days; day++) {
        if (day == st.wDay && month == st.wMonth && year == st.wYear) {
            set_col(C_OK);
            printf("%2d ", day);
            set_col(C_RESET);
        } else {
            printf("%2d ", day);
        }
        if ((first_day + day) % 7 == 0) printf("\n");
    }
    printf("\n");
}

// [NEW] Pomodoro
void cmd_pomodoro(char** args, int c) {
    printf("Starting 25 minute focus timer...\n");
    // Mocking 25 mins as 25 seconds for demo
    for(int i=25; i>0; i--) {
        printf("\rTime left: %d min ... ", i);
        tnr_sleep(100); // Fast forward
    }
    printf("\nRING RING! Take a break.\n");
    printf("\a");
}

// [NEW] Journal
void cmd_journal(char** args, int c) {
    if (c < 2) { printf("Usage: journal <entry>\n"); return; }
    FILE *f = fopen("journal.txt", "a");
    SYSTEMTIME st; GetLocalTime(&st);
    fprintf(f, "[%02d/%02d %02d:%02d] ", st.wDay, st.wMonth, st.wHour, st.wMinute);
    for(int i=1; i<c; i++) fprintf(f, "%s ", args[i]);
    fprintf(f, "\n");
    fclose(f);
    printf("Journal entry saved.\n");
}

// [NEW] Theme
void cmd_theme(char** args, int c) {
    if (c < 2) { printf("Themes: hack, retro, ocean\n"); return; }
    if (strcmp(args[1], "hack") == 0) system("color 0A");
    else if (strcmp(args[1], "retro") == 0) system("color 0E");
    else if (strcmp(args[1], "ocean") == 0) system("color 1B");
    else system("color 07");
    printf("Theme set.\n");
}

void cmd_calc(char** args, int c) {
    if (c < 4) { printf("Usage: calc 5 + 5\n"); return; }
    double a = atof(args[1]);
    double b = atof(args[3]);
    char op = args[2][0];
    double res = 0;
    if (op == '+') res = a + b;
    else if (op == '-') res = a - b;
    else if (op == '*') res = a * b;
    else if (op == '/') res = (b!=0) ? a/b : 0;
    else if (op == '%') res = (int)a % (int)b;
    else if (op == '^') { res = 1; for(int i=0; i<(int)b; i++) res*=a; } // Simple pow
    printf("Result: %f\n", res);
}

void cmd_clear(char** args, int c) { system("cls"); }

void cmd_echo(char** args, int c) {
    for(int i=1; i<c; i++) printf("%s ", args[i]);
    printf("\n");
}

void cmd_color(char** args, int c) {
    if (c < 2) { print_usage("color", "<0-15>"); return; }
    int color = atoi(args[1]);
    if (color < 0 || color > 15) { print_error("Color must be 0-15."); return; }
    set_col(color);
}

void cmd_exit(char** args, int c) { running = 0; }

void cmd_history(char** args, int c) {
    print_header("COMMAND HISTORY");
    if (history_count == 0) {
        printf("No commands in history.\n");
        return;
    }
    int start = (history_count > MAX_HISTORY) ? (history_count - MAX_HISTORY) : 0;
    int idx = 1;
    for (int i = start; i < history_count; i++) {
        set_col(C_INFO);
        printf("%3d  ", idx++);
        set_col(C_RESET);
        printf("%s\n", cmd_history[i % MAX_HISTORY]);
    }
}

void cmd_run(char** args, int c) { 
    if (c < 2) { print_usage("run", "<command>"); return; }
    
    // Security warning
    set_col(C_WARN);
    printf("Executing system command: %s\n", args[1]);
    set_col(C_RESET);
    
    system(args[1]); 
}

// [NEW] Man - Full implementation
void cmd_man(char** args, int c) {
    if (c < 2) { print_usage("man", "<command>"); return; }
    
    const char* cmd = args[1];
    
    for (int i = 0; man_pages[i].name != NULL; i++) {
        if (strcmp(man_pages[i].name, cmd) == 0) {
            print_header("MANUAL");
            set_col(C_INFO);
            printf("Command: ");
            set_col(C_OK);
            printf("%s\n", man_pages[i].name);
            
            if (strlen(man_pages[i].usage) > 0) {
                set_col(C_INFO);
                printf("Usage:   ");
                set_col(C_RESET);
                printf("%s %s\n", man_pages[i].name, man_pages[i].usage);
            }
            
            set_col(C_INFO);
            printf("\nDescription:\n");
            set_col(C_RESET);
            printf("  %s\n", man_pages[i].desc);
            return;
        }
    }
    
    print_error("No manual entry for that command.");
}
