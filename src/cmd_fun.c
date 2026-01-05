#include "tnr.h"

// Matrix Rain Effect - Enhanced
void cmd_matrix(char** args, int c) {
    system("cls");
    hide_cursor();
    
    int width = 80;
    int height = 25;
    int drops[80];
    char chars[80];
    
    for(int i = 0; i < width; i++) {
        drops[i] = rand() % height;
        chars[i] = (rand() % 93) + 33;
    }
    
    set_col(C_INFO);
    printf("[ MATRIX - Press any key to exit ]\n");
    
    while(!_kbhit()) {
        for(int i = 0; i < width; i++) {
            int x = i;
            int y = drops[i];
            
            // Draw bright head
            gotoxy(x, y + 1);
            set_col(C_WHITE);
            chars[i] = (rand() % 93) + 33;
            printf("%c", chars[i]);
            
            // Draw trail
            if (y > 0) {
                gotoxy(x, y);
                set_col(C_OK);
                printf("%c", (rand() % 93) + 33);
            }
            
            // Fade old chars
            if (y > 3) {
                gotoxy(x, y - 3);
                set_col(C_HACK);
                printf("%c", (rand() % 93) + 33);
            }
            
            // Clear very old
            if (y > 8) {
                gotoxy(x, y - 8);
                printf(" ");
            }
            
            drops[i]++;
            if (drops[i] > height + 8) {
                drops[i] = 0;
            }
        }
        tnr_sleep(40);
    }
    _getch();
    show_cursor();
    set_col(C_RESET);
    system("cls");
}

// Snake Game - Enhanced with trail
void cmd_snake(char** args, int c) {
    system("cls");
    hide_cursor();
    
    #define SNAKE_LEN 100
    int sx[SNAKE_LEN], sy[SNAKE_LEN];
    int len = 3;
    sx[0] = 10; sy[0] = 10;
    sx[1] = 9; sy[1] = 10;
    sx[2] = 8; sy[2] = 10;
    
    int fx = 15, fy = 15;
    int score = 0;
    int dx = 1, dy = 0;
    char key = 0;
    
    while(1) {
        gotoxy(0, 0);
        set_col(C_INFO);
        printf("+------------------[ SNAKE ]------------------+\n");
        set_col(C_OK);
        printf("| Score: %-5d                    WASD to move |\n", score);
        set_col(C_INFO);
        printf("+---------------------------------------------+\n");
        set_col(C_RESET);
        
        // Draw border and game
        for(int i = 0; i < 20; i++) {
            gotoxy(0, i + 3);
            for(int j = 0; j < 40; j++) {
                if (i == 0 || i == 19) {
                    set_col(C_INFO);
                    printf("=");
                } else if (j == 0 || j == 39) {
                    set_col(C_INFO);
                    printf("|");
                } else {
                    int isSnake = 0;
                    for(int k = 0; k < len; k++) {
                        if (i == sy[k] && j == sx[k]) {
                            if (k == 0) {
                                set_col(C_OK);
                                printf("@");
                            } else {
                                set_col(C_HACK);
                                printf("o");
                            }
                            isSnake = 1;
                            break;
                        }
                    }
                    if (!isSnake) {
                        if (i == fy && j == fx) {
                            set_col(C_ERR);
                            printf("*");
                        } else {
                            printf(" ");
                        }
                    }
                }
            }
        }
        
        if (_kbhit()) {
            key = _getch();
            if (key == 'w' && dy != 1) { dx = 0; dy = -1; }
            if (key == 's' && dy != -1) { dx = 0; dy = 1; }
            if (key == 'a' && dx != 1) { dx = -1; dy = 0; }
            if (key == 'd' && dx != -1) { dx = 1; dy = 0; }
            if (key == 'x' || key == 'q') break;
        }
        
        // Move snake
        for(int i = len - 1; i > 0; i--) {
            sx[i] = sx[i-1];
            sy[i] = sy[i-1];
        }
        sx[0] += dx;
        sy[0] += dy;
        
        // Check fruit
        if (sx[0] == fx && sy[0] == fy) {
            score += 10;
            if (len < SNAKE_LEN - 1) len++;
            fx = (rand() % 36) + 2;
            fy = (rand() % 16) + 2;
            printf("\a");
        }
        
        // Check collision
        if (sx[0] <= 0 || sx[0] >= 39 || sy[0] <= 0 || sy[0] >= 19) {
            gotoxy(15, 12);
            set_col(C_ERR);
            printf("GAME OVER!");
            tnr_sleep(2000);
            break;
        }
        
        // Self collision
        for(int i = 1; i < len; i++) {
            if (sx[0] == sx[i] && sy[0] == sy[i]) {
                gotoxy(15, 12);
                set_col(C_ERR);
                printf("GAME OVER!");
                tnr_sleep(2000);
                goto end_snake;
            }
        }
        
        tnr_sleep(80);
    }
    end_snake:
    show_cursor();
    set_col(C_RESET);
    gotoxy(0, 24);
    printf("\nFinal Score: %d\n", score);
}

void cmd_weather(char** args, int c) {
    const char* w[] = {"Sunny", "Rainy", "Cloudy", "Cyber-Storm", "Nuclear Winter", "Acid Rain", "Partly Cloudy", "Thunderstorms"};
    const char* icons[] = {"\xF0\x9F\x8C\x9E", "\xF0\x9F\x8C\xA7", "\xE2\x98\x81", "\xE2\x9A\xA1", "\xE2\x9D\x84", "\xE2\x98\xA2", "\xF0\x9F\x8C\xA4", "\xE2\x9B\x88"};
    int idx = rand() % 8;
    print_header("WEATHER");
    printf("Condition: %s\n", w[idx]);
    printf("Temperature: %d C (%d F)\n", rand() % 45 - 5, (rand() % 45 - 5) * 9 / 5 + 32);
    printf("Humidity: %d%%\n", rand() % 100);
    printf("Wind: %d km/h\n", rand() % 50);
}

void cmd_joke(char** args, int c) {
    const char* jokes[] = {
        "There are 10 types of people: those who understand binary, and those who don't.",
        "Why do Java programmers wear glasses? Because they don't C#.",
        "A SQL query walks into a bar, walks up to two tables and asks... 'Can I join you?'",
        "Why did the programmer quit his job? Because he didn't get arrays (a raise).",
        "A programmer puts two glasses on his bedside table before going to sleep. A full one, in case he gets thirsty, and an empty one, in case he doesn't.",
        "There are only 3 hard things in programming: naming things, cache invalidation, and off-by-one errors.",
        "!false - It's funny because it's true.",
        "A SQL statement walks into a bar and sees two tables. It walks up to them and asks 'Can I join you?'",
        "Why do programmers prefer dark mode? Because light attracts bugs.",
        "What's the object-oriented way to become wealthy? Inheritance."
    };
    set_col(C_INFO);
    printf("%s\n", jokes[rand() % 10]);
    set_col(C_RESET);
}

void cmd_fortune(char** args, int c) {
    const char* quotes[] = {
        "The computer was born to solve problems that did not exist before.",
        "It's not a bug, it's a feature.",
        "Talk is cheap. Show me the code. - Linus Torvalds",
        "First, solve the problem. Then, write the code. - John Johnson",
        "Any fool can write code that a computer can understand. Good programmers write code that humans can understand. - Martin Fowler",
        "Code is like humor. When you have to explain it, it's bad.",
        "Simplicity is the soul of efficiency. - Austin Freeman",
        "The best error message is the one that never shows up. - Thomas Fuchs",
        "Programming isn't about what you know; it's about what you can figure out. - Chris Pine",
        "The only way to learn a new programming language is by writing programs in it. - Dennis Ritchie"
    };
    print_header("FORTUNE");
    set_col(C_WARN);
    printf("\"%s\"\n", quotes[rand() % 10]);
    set_col(C_RESET);
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
