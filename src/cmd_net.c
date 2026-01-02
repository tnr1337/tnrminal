#include "tnr.h"

// [NEW] Ping Simulation
void cmd_ping(char** args, int c) {
    if (c < 2) { printf("Usage: ping <host>\n"); return; }
    printf("Pinging %s [10.0.0.1] with 32 bytes of data:\n", args[1]);
    
    for(int i=0; i<4; i++) {
        int ms = (rand() % 100) + 20;
        tnr_sleep(500); // Wait bit
        if (rand() % 10 == 0) {
            printf("Request timed out.\n");
        } else {
            printf("Reply from 10.0.0.1: bytes=32 time=%dms TTL=128\n", ms);
        }
    }
}

// [NEW] IP
void cmd_ip(char** args, int c) {
    printf("\nEthernet adapter Ethernet:\n\n");
    printf("   Connection-specific DNS Suffix  . : local\n");
    printf("   Link-local IPv6 Address . . . . . : fe80::a1b2:c3d4:e5f6%%12\n");
    printf("   IPv4 Address. . . . . . . . . . . : 192.168.1.%d\n", rand()%254 + 1);
    printf("   Subnet Mask . . . . . . . . . . . : 255.255.255.0\n");
    printf("   Default Gateway . . . . . . . . . : 192.168.1.1\n\n");
}

// [NEW] Curl
void cmd_curl(char** args, int c) {
    if (c < 2) { printf("Usage: curl <url>\n"); return; }
    printf("Connecting to %s...\n", args[1]);
    tnr_sleep(500);
    printf("HTTP/1.1 200 OK\n");
    printf("Content-Type: text/html\n\n");
    
    // Progress Bar
    printf("[--------------------] 0%%");
    for(int i=0; i<=20; i++) {
        printf("\r[");
        for(int j=0; j<i; j++) printf("#");
        for(int j=i; j<20; j++) printf("-");
        printf("] %d%%", i*5);
        tnr_sleep(50 + rand()%100);
    }
    printf("\n\n<!DOCTYPE html><html><body><h1>Response</h1></body></html>\n");
}
