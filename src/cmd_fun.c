#include "tnr.h"

// [NEW] Matrix Rain Effect
void cmd_matrix(char** args, int c) {
    system("cls");
    set_col(C_HACK);
    
    // Simple matrix effect: random columns
    int width = 80;
    int cols[80];
    for(int i=0; i<width; i++) cols[i] = rand() % 20;

    printf("Press any key to stop...\n");
    
    while(!_kbhit()) {
        for(int i=0; i<width; i++) {
             // 1 in 10 chance to draw char
            if (rand() % 10 > 8) {
                // Random Katakana-ish or ASCII
                char ch = (rand() % 93) + 33;
                printf("%c", ch);
            } else {
                printf(" ");
            }
        }
        // Small delay
        tnr_sleep(50);
    }
    _getch(); // Clear key
    set_col(C_RESET);
    system("cls");
}

// [NEW] Snake Game (Very Basic)
void cmd_snake(char** args, int c) {
    system("cls");
    printf("SNAKE (WASD to move, X to quit)\n");
    
    int x=10, y=10;
    int fx=15, fy=15; // Fruit
    int score = 0;
    char key = 'd';
    
    while(key != 'x') {
        system("cls");
        printf("Score: %d\n", score);
        
        // Draw Field (20x20)
        for(int i=0; i<20; i++) {
            for(int j=0; j<20; j++) {
                if (i==0 || i==19 || j==0 || j==19) printf("#");
                else if (i==y && j==x) printf("O");
                else if (i==fy && j==fx) printf("*");
                else printf(" ");
            }
            printf("\n");
        }
        
        if (_kbhit()) {
            key = _getch();
        }
        
        if (key == 'w') y--;
        if (key == 's') y++;
        if (key == 'a') x--;
        if (key == 'd') x++;
        
        if (x == fx && y == fy) {
            score++;
            fx = (rand() % 18) + 1;
            fy = (rand() % 18) + 1;
        }
        
        if (x<=0 || x>=19 || y<=0 || y>=19) {
            printf("GAME OVER!\n");
            break;
        }
        
        tnr_sleep(100);
    }
    printf("Final Score: %d\n", score);
}

void cmd_weather(char** args, int c) {
    const char* w[] = {"Sunny", "Rainy", "Cloudy", "Cyber-Storm", "Nuclear Winter", "Acid Rain"};
    printf("Current Weather: %s | Temp: %d C\n", w[rand()%6], rand()%45);
}

void cmd_joke(char** args, int c) {
    const char* jokes[] = {
        "There are 10 types of people: those who understand binary, and those who don't.",
        "Why do Java programmers wear glasses? Because they don't C#.",
        "A SQL query walks into a bar, walks up to two tables and asks... 'Can I join you?'"
    };
    printf("%s\n", jokes[rand()%3]);
}

void cmd_fortune(char** args, int c) {
    const char* quotes[] = {
        "The computer was born to solve problems that did not exist before.",
        "It's not a bug, it's a feature.",
        "Talk is cheap. Show me the code."
    };
    printf("Fortune: %s\n", quotes[rand()%3]);
}

void cmd_selfdestruct(char** args, int c) {
    set_col(C_ERR);
    printf("INITIATING SELF DESTRUCT SEQUENCE...\n");
    for(int i=5; i>0; i--) {
        printf("%d...\n", i);
        tnr_sleep(1000);
    }
    printf("BOOM! (Just kidding)\n");
    set_col(C_RESET);
}

void cmd_rand(char** args, int c) {
    printf("%d\n", rand());
}

void cmd_dice(char** args, int c) {
    printf("Rolled: %d\n", (rand() % 6) + 1);
}

void cmd_beep(char** args, int c) {
    printf("\aBEEP!\n"); // Standard bell
}
