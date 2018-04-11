#include "kernel.h"

void main() {
    int i, j;
    char curdir;
    char argc;
    char argv[4][16];
    int succ;
    char chain[MAX_DIRS];
    int count = 0;
    char buff[MAX_FILENAME + 1];
    char dirs[SECTOR_SIZE];
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    for (i = 0; i < argc; i++) {
        interrupt(0x21, 0x23, i, argv[i], 0);
    }
    interrupt(0x21, 0x02, dirs, DIRS_SECTOR);

    while (curdir != 0xFF) {
        chain[count] = curdir;
        count++;
        curdir = dirs[curdir * ENTRY_LENGTH];
    }
    interrupt(0x21, 0x0, "/", 0, 0);
    for (i = count - 1; i >= 0; i--) {
        j = 0;
        while ((j < MAX_FILENAME) && (dirs[chain[i] * ENTRY_LENGTH + 1 + j] != '\0')) {
            buff[j] = dirs[chain[i] * ENTRY_LENGTH + 1 + j];
            j++;
        }
        buff[j] = '\0';
        interrupt(0x21, 0x0, buff, 0, 0);
        interrupt(0x21, 0x0, "/", 0, 0);
    }
    interrupt(0x21, 0x0, "\n", 0, 0);
    interrupt(0x21, (0x00 << 8) | 0x07, &succ, 0, 0);
}