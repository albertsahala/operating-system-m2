#include "kernel.h"

char cmpArray(char * arr1, char * arr2, int length);
int div(int a, int b);

void main() {
    int i, j, k;
    char curdir;
    char argc;
    char argv[4][32];
    int succ;
    char buff[SECTOR_SIZE * MAX_SECTORS];
    char l_buffer[128];
    int sectors;
    char mode = 0;
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    for (i = 0; i < argc; i++) {
        interrupt(0x21, 0x23, i, argv[i], 0);
    }
    if (argc > 0) {
        if (argc > 1) {
            if (cmpArray("-w", argv[1], 2)) {
                mode = 1;
            }
        }
        if (mode == 0) {
            // Output the content of a file.
            interrupt(0x21, (curdir << 8) | 0x04, buff, argv[0], &succ);
            if (succ == SUCCESS) {
                interrupt(0x21, 0x0, buff, 0, 0);      
                interrupt(0x21, 0x0, "\n", 0, 0);      
            } else {
                interrupt(0x21, 0x0, "file not found\n", 0, 0);
            }
        } else if (mode == 1) {    
            for (i = 0; i < SECTOR_SIZE * MAX_SECTORS; i++) {
                buff[i] = '\0';
            }
            interrupt(0x21, 0x0, "~~ empty line to end ~~\n", 0, 0);
            k = 0;
            interrupt(0x21, 0x01, l_buffer, 0, 0);
            while (l_buffer[0] != '\0') {
                j = 0;
                while (l_buffer[j] != '\0') {
                    buff[k] = l_buffer[j];
                    j++;
                    k++;
                }
                buff[k] = '\n';
                k++;
                interrupt(0x21, 0x01, l_buffer, 0, 0);
            }
            buff[k - 1] = '\0';
            interrupt(0x21, 0x0, "~~ saving ~~\n", 0, 0);
            sectors = div(k, SECTOR_SIZE) + 1;
            interrupt(0x21, (curdir << 8) | 0x05, buff, argv[0], &sectors);
        }
        
    }
    interrupt(0x21, (0x00 << 8) | 0x07, &succ, 0, 0);
}

int div(int a, int b) {
    int q = 0;
    while(q*b <=a) {
        q = q+1;
    }
    return q-1;
}

char cmpArray(char * arr1, char * arr2, int length) {
    int i = 0;
    char equal = TRUE;
    while ((equal) && (i < length)) {
        equal = arr1[i] == arr2[i];
        if (equal) {
            if (arr1[i] == '\0') {
                i = length;
            }
        }
        i++;
    }
    return equal;
}