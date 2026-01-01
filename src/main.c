#include <stdio.h>
#include <windows.h>

int main() {
    // Get handle to the output buffer
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // Set text color to Green (Classic Terminal)
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    
    printf("TNRM1N4L - System Initialization...\n");
    printf("[KERNEL] Loading Core Modules...\n");
    printf("[KERNEL] Memory: OK\n");
    printf("[KERNEL] CPU: OK\n");
    
    // Mimic some loading time
    Sleep(500);
    
    printf("\nWelcome to TNRM1N4L v0.0.1 (Alpha)\n");
    printf("Ready for input.\n");
    
    // Simple loop to keep window open
    while(1) {
        Sleep(100);
    }
    
    return 0;
}
