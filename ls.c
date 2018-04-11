#include "kernel.h"

void main(){
    char dir[SECTOR_SIZE];
    char file[SECTOR_SIZE];
    char curdir;
    int i;
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x02, dir, DIRS_SECTOR, 0);
    interrupt(0x21, 0x02, file, FILES_SECTOR, 0);
    interrupt(0x21, 0x00, "Daftar File : \r", 0, 0);
    for(i=0;i<MAX_FILES;i++){
        if(file[i*DIR_ENTRY_LENGTH] == curdir && file[i*DIR_ENTRY_LENGTH+1] != '\0'){
            interrupt(0x21, 0x00, file+(i*DIR_ENTRY_LENGTH+1), 0, 0);
            interrupt(0x21, 0x00, "\r", 0, 0);
        }
    }
    interrupt(0x21, 0x00, "Daftar Folder :\r\n", 0, 0);
    for(i=0;i<MAX_FILES;i++){
        if(dir[i*DIR_ENTRY_LENGTH] == curdir && dir[i*DIR_ENTRY_LENGTH+1] != '\0'){
            interrupt(0x21, 0x00, dir+(i*DIR_ENTRY_LENGTH+1), 0, 0);
            interrupt(0x21, 0x00, "\r\n", 0, 0);
        }
    }
    interrupt(0x21, 0x07, &i, 0, 0);
}