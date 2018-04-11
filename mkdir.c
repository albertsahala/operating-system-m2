#include "kernel.h"

char cmpArray(char * arr1, char * arr2, int length);

void main() {
    int i;
    char curdir;
    char argc;
    char argv[4][32];
    int succ;
    char buff[MAX_FILENAME + 1];
    char dest;
    char mode = 0;
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    for (i = 0; i < argc; i++) {
        interrupt(0x21, 0x23, i, argv[i], 0);
    }
    if (argc > 0) {
        interrupt(0x21, (curdir << 8) | 0x08, argv[0], &succ, 0);
        if (succ == ALREADY_EXISTS) {
            interrupt(0x21, 0x0, "directory exists.\n", 0, 0);            
        } else if (succ == NOT_FOUND) {
            interrupt(0x21, 0x0, "directory not found.\n", 0, 0); 
        }
    }
    interrupt(0x21, (0x00 << 8) | 0x07, &succ, 0, 0);
}