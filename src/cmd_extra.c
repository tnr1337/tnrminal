#include "tnr.h"

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

// [NEW] Calendar
void cmd_calendar(char** args, int c) {
    printf("   MTWTFSS   \n");
    printf("   1 2 3 4   \n");
    printf(" 5 6 7 8 9 10 11\n");
    printf("12 13 14 15 16 17 18\n");
    printf("19 20 21 22 23 24 25\n");
    printf("26 27 28 29 30 31   \n");
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
    if (c < 2) return;
    set_col(atoi(args[1]));
}
void cmd_exit(char** args, int c) { running = 0; }
void cmd_history(char** args, int c) { printf("Feature pending (Session storage).\n"); }
void cmd_run(char** args, int c) { if(c>1) system(args[1]); }

// [NEW] Man
void cmd_man(char** args, int c) {
    if (c < 2) { printf("Usage: man <cmd>\n"); return; }
    // Ideally look up in dispatch table
    printf("Manual for %s: (Detailed help pending)\n", args[1]);
}
