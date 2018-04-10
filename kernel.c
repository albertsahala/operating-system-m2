#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_FILES 16
#define MAX_FILENAME 12
#define MAX_SECTORS 20
#define DIR_ENTRY_LENGTH 32
#define MAP_SECTOR 1
#define DIR_SECTOR 2
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define NOT_FOUND -1
#define INSUFFICIENT_DIR_ENTRIES -1
#define INSUFFICIENT_ENTRIES -3
#define EMPTY 0x00
#define USED 0xFF
#define ARGS_SECTOR 512

void handleInterrupt21 (int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void clear(char *buffer, int length);
void writeFile(char *buffer, char *filename, int *sectors);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex);
void executeProgram(char *path, int segment, int *result, char parentIndex);
void terminateProgram(int *result);
void writeText(char c, int i, int color);
void makeDirectory(char *path, int *result, char parentIndex);
void deleteDirectory(char *path, int *success, char parentIndex);
void putArgs (char curdir, char argc, char **argv);
void getCurdir (char *curdir);
void getArgc (char *argc);
void getArgv (char index, char *argv);
void splashScreen();

int main() {
  int sukses;
  char *hasil;
  int i = 0;
  int j = 0;

  while (i <= 14){
    if (j == 80){
      j = 0;
      i++;
    }
    putInMemory(0xB000, 0x8000 + ((80*i + j-1)*2), ' ' );
    putInMemory(0xB000, 0x8001 + ((80*i + j-1)*2), 0x3 );
    j++;
  }

  //splashScreen();
  makeInterrupt21();
  interrupt(0x21, 0x6, "k", 0x2000, &sukses);

  while (1);
}

void handleInterrupt21 (int AX, int BX, int CX, int DX) {
  char AL, AH;
  AL = (char) (AX);
  AH = (char) (AX >> 8);

  switch (AL) {
    case 0x00:
      printString(BX);
      break;
    case 0x01:
      readString(BX);
      break;
    case 0x02:
      readSector(BX, CX);
      break;
    case 0x03:
      writeSector(BX, CX);
      break;
    case 0x04:
      readFile(BX, CX, DX, AH);
      break;
    case 0x05:
      writeFile(BX, CX, DX, AH);
      break;
    case 0x06:
      executeProgram(BX, CX, DX, AH);
      break;
    case 0x07:
      terminateProgram(BX);
      break;
    case 0x08:
      makeDirectory(BX, CX, AH);
      break;
    case 0x09:
      deleteFile(BX, CX, AH);
      break;
    case 0x0A:
      deleteDirectory(BX, CX, AH);
      break;
    case 0x20:
      putArgs(BX, CX);
      break;
    case 0x21:
      getCurdir(BX);
      break;
    case 0x22:
      getArgc(BX);
      break;
    case 0X23:
      getArgv(BX, CX);
      break;
    default:
    printString("Invalid interrupt");
  }
}

void printString(char *string) {
  int i = 0;
  while(string[i] != '\0') {
    int ch = string[i];
    // interrupt #, AX, BX, CX, DX
    interrupt(0x10, 0xE00+ch, 0, 0, 0);
    i++;
  }
}

void readString(char *string) {
  int dashn = 0xa;
  int endStr = 0x0;
  int enter = 0xd;
  int backsp = 0x8;
  int dashr = 0xd;
  int loop = 1;
  int count = 2;
  string[0] = dashr;
  string[1] = dashn;
  while(loop){
    /* Call interrupt 0x16 */
    /* interrupt #, AX, BX, CX, DX */
    int ascii = interrupt(0x16,0,0,0,0);
    if (ascii == enter){              
      string[count] = 0x0;
      string[count+1] = dashr;
      string[count+2] = dashn;
      return;
    } else if (ascii == backsp){
      if (count > 1){
        string[count] = 0x0;
        count--;
        interrupt(0x10,0xe*256+0x8,0,0,0);
        count++;
        interrupt(0x10,0xe*256+0x0,0,0,0);
        count--;
        interrupt(0x10,0xe*256+0x8,0,0,0);          
      }
    } else{
      string[count] = ascii;
      interrupt(0x10, 0xe*256+ascii, 0, 0, 0);
      count++;
    }     
  }
}

int mod(int a, int b) {
  while (a >= b) {
    a = a - b;
  }
  return a;
}

int div(int a, int b) {
  int q = 0;
  while (q*b <= a) {
    q = q + 1;
  }
  return q-1;
}

void readSector(char *buffer, int sector) {
  interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector) {
  interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void clear(char *buffer, int length) {
  int i;
  for(i = 0; i < length; ++i) {
    buffer[i] = EMPTY;
  }
}

void searchDirectory_V2(char *dir, char *path, int *result, char *parentIndex){
  int i;
  int j;
  int check;

  check = 0;
  for (i = 0; i < 32; i++){ //Searching if there is a directory equals to path
    if (dir[i*16] == *parentIndex){   //parentIndex is same, checking the filenames
      check = 1;
      j = 0;
      while (path[j] != '\0'){  //Checking the directory name
        if (path[j] != dir[i*16+1+j]){  //Directory name is not the same, break
          check = 0;
          break;
        } else {
          j++;
        }
      }

      if ((check) && (dir[i*16+1+j] == '\0')){  //Name is the same, so as the length
        *parentIndex = i*16;
        break;
      }
    }
  }

  *result = check;
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex){
  char data[SECTOR_SIZE];
  char buffer_data[SECTOR_SIZE];
  char directory_name[15];
  int check;
  int count;
  int free_idx;
  int i;
  int j;

  check = 0;
  count = 0;
  readSector(data,0x100);
  for (i = 0; i < SECTOR_SIZE; i++){
    if (data[i] == '\0'){
      count++;
    }
    if ((j+1) == *sectors){
      check = 1;
      break;
    }
  }

  if (check != 1){
    readSector(data,0x102);
    check = -3;
    for (i = 0; i < 32; i++){
      if (data[i*16] == '\0'){
        free_idx = i;
        break;
      }
    }
    if (check != 1){
      *sectors = check;
    } else {
      readSector(data,0x101);

      for (i = 0; i < 15; i++){
        directory_name[i] = '\0';
      }

      i = 1;
      while (path[i] != '\0'){  //Iterating until name of directory to be deleted reached
        j = 0;
        while ((path[i] != '/') && (path[i] != '\0')){  //Parsing the path to a new array of string
          directory_name[j] = path[i];
          i++;
          j++;
        }
        directory_name[i] = '\0';
        searchDirectory_V2(data,directory_name,&check,&parentIndex); //Searching whether the current path exists
        if (check){   //Path exist, continue checking next path
          for (j = 0; j < 15; j++){
            directory_name[j] = '\0';
          }
          i++;
        } else {  //Path does not exist, abort send error -1
          check = -1;
          break;
        }
      }

      if (check != 1){
        *sectors = check;
      } else {
        readSector(data,0x102);
        searchDirectory_V2(data,directory_name,&check,&parentIndex);

        if (!(check)){
          data[free_idx*16] = parentIndex;
          i = 0;
          while(directory_name[i] != '\0'){
            data[free_idx*16+i+1] = directory_name[i];
            i++;
          }
          writeSector(data,0x102);

          readSector(data,0x100);
          for (i = 0; i < *sectors; i++){
            if (data[i] == '\0'){
              data[i] = 0xFF;
              for (j = 0; j < SECTOR_SIZE; j++){
                buffer_data[i] = '\0';
                buffer_data[i] = buffer[i * SECTOR_SIZE + j];
                writeSector(buffer_data,i);
              }
            }

            if (i > 15){
              break;
            }
          }
          writeSector(data,0x100);
        } else {
          *sectors = -2;
        }
      }
    }
  } else {
    *sectors = check;
  }
}

void deleteFile(char *path, int *result, char parentIndex){
  char directory_name[15];
  char data[SECTOR_SIZE];
  int check;
  int i;
  int j;

  for (i = 0; i < 15; i++){
    directory_name[i] = '\0';
  }

  readSector(data,0x101);
  i = 1;
  while (path[i] != '\0'){  //Iterating until name of directory to be deleted reached
    j = 0;
    while ((path[i] != '/') && (path[i] != '\0')){  //Parsing the path to a new array of string
      directory_name[j] = path[i];
      i++;
      j++;
    }
    directory_name[i] = '\0';
    if (path[i] != '\0'){
      searchDirectory_V2(data,directory_name,&check,&parentIndex); //Searching whether the current path exists
      if (check){   //Path exist, continue checking next path
        for (j = 0; j < 15; j++){
          directory_name[j] = '\0';
        }
        i++;
      } else {  //Path does not exist, abort send error -1
        check = -1;
        break;
      }
    }
  }

  if (check != 1){
    readSector(data,0x102);
    searchDirectory_V2(data,directory_name,&check,&parentIndex);

    if (check){
      data[parentIndex*16] = '\0';

      readSector(data,0x103);
      i = 0;
      while (data[parentIndex*16+i] != '\0'){
        j = data[parentIndex*16+i];
        readSector(data,0x100);
        data[j] = '\0';
        i++;
        readSector(data,0x103);
      }

      check = 0;
    } else {
      check = -1;
    }
  }

  *result = check;
}

void readFile(char *buffer, char *path, int *result, char parentIndex){
  char data[512];
  char file_data[512];
  char directory_name[15];
  int check;
  int i;
  int j;
  int k;

  i = 1;
  check = 0;
  readSector(data,0x101);
  while (data[i] != '\0'){
    j = 0;
    while ((path[i] != '/') && (path[i] != '\0')){
      directory_name[j] = path[i];
      i++;
      j++;
    }
    directory_name[i] = '\0';
    if (path[i] != '\0'){
      searchDirectory_V2(data,directory_name,&check,&parentIndex);
      if (check){
        for (j = 0; j < 15; j++){
          directory_name[j] = '\0';
        }
        i++;
      } else {
        check = -1;
        break;
      }
    }
  }

  if (check == 0){
    readSector(data,0x102);
    searchDirectory_V2(data,directory_name,&check,&parentIndex);

    if (check){
      readSector(data,0x103);
      k = 0;
      i = 0;
      while(data[parentIndex*16 + i] != '\0'){
        readSector(file_data,data[parentIndex*16+i]);
        for(j = 0; j < SECTOR_SIZE; j++){
          buffer[k] = file_data[j];
          k++;
        }
        i++;
      }
      check = 0;
    } else {
      check = -1;
    }
  }

  *result = check;
}

void executeProgram(char *path, int segment, int *result, char parentIndex) {
  char buffer[16*SECTOR_SIZE];

  clear(buffer,16*SECTOR_SIZE);
  readFile(buffer,path,result,parentIndex);

  if (*result == 0) {
    int i;
    for(i = 0; i < 16*SECTOR_SIZE; i++) {
      putInMemory(segment, i, buffer[i]);
    } 
    launchProgram(segment);
  }
}

void terminateProgram(int *result){
  
}

void deleteDirectories(char *dir, char parentIndex){
  int ArrayOfIndex[15];
  int i;
  int j;

  for (i = 0; i < 15; i++){ //Setting up array for saving directory indexes
    ArrayOfIndex[i] = -999;
  }

  j = 0;
  for (i = 0; i < 32; i++){
    if (dir[i*16] == parentIndex){  //Put into array if the directory parent index is the same
      ArrayOfIndex[j] = i*16;
      dir[i*16] = '\0'; //Delete the directory
      j++;
    }
  }

  i = 0;
  while (ArrayOfIndex[i] != -999){  //Continue the rest of directories inside of array recursively
    deleteDirectories(dir,ArrayOfIndex[i]);
    i++;
  }
}

void deleteDirectory(char* dir, char *path, int *success, char parentIndex){
  char directory_name[15];
  char dir[SECTOR_SIZE];
  int i;
  int j;
  int check;
  int count = 0;

  readSector(dir,0x101);

  i = 1;
  while (path[i] != '\0'){  //Iterating until name of directory to be deleted reached
    j = 0;
    while ((path[i] != '/') && (path[i] != '\0')){  //Parsing the path to a new array of string
      directory_name[j] = path[i];
      i++;
      j++;
    }
    directory_name[i] = '\0';
    searchDirectory_V2(dir,directory_name,&check,&parentIndex); //Searching whether the current path exists
    if (check){   //Path exist, continue checking next path
      for (j = 0; j < 15; j++){
        directory_name[j] = '\0';
      }
      i++;
    } else {  //Path does not exist, abort send error -1
      check = -1;
      break;
    }
  }

  if (check == 1){  //No error generated so far, continue to delete the directory
    dir[parentIndex] = '\0';
    check = 0;
    deleteDirectories(dir,parentIndex); //Delete all of the directory inside of current directory
  }

  writeSector(dir,0x101);
  *success = check; //Send program execution status
}

void makeDirectory(char *path, int *result, char parentIndex){
  char directory_name[15];
  char dir[SECTOR_SIZE];
  int i;
  int j;
  int check;
  int free_idx;

  readSector(dir,0x101);

  for (j = 0; j < 15; j++){ //Emptying the array
    directory_name[j] = '\0';
  }

  check = -3;
  for (i = 0; i < 32; i++){ //Checking if there is a space available for new directory
    if (dir[i*16] == '\0'){   //There is a space, break and save the directory location
      free_idx = i;
      check = 0;
      break;
    }
  }

  i = 1;
  while ((path[i] != '\0') && (check != -3)){   //Checking the preceding path before creating new directory
    j = 0;
    while ((path[i] != '/') && (path[i] != '\0')){    //Parsing path by '/' and '\0'
      directory_name[j] = path[i];
      i++;
      j++;
    }
    directory_name[i] = '\0';
    if (path[i] != '\0'){ //The directory_name is not the directory want to be made
      searchDirectory_V2(dir,directory_name,&check,&parentIndex); //Check if the directory exists
      
      if (check){ //directory exists, repeat with the next directory
        for (j = 0; j < 15; j++){
          directory_name[j] = '\0';
        }
        i++;
      } else {  //directory does not exist, abort
        check = -1;
        break;
      }
    } else {
      check = 0;
    }
  }

  if (check == 0){  //No error generated so far, continue
    searchDirectory_V2(dir,directory_name,&check,&parentIndex); //Check if there is already a directory with same name
    if (!(check)){  //No directory exist, create new
      dir[free_idx*16] = parentIndex;

      i = 0;
      while(directory_name[i] != '\0'){
        dir[free_idx*16+i+1] = directory_name[i];
        i++;
      }
      check = 0;
      writeSector(dir,0x101);
    } else {  //Directory already exist, abort
      check = -2;
    }
  }

  *result = check;
}

void putArgs (char curdir, char argc, char **argv) {
  char args[SECTOR_SIZE];
  int i, j, p;
  clear(args, SECTOR_SIZE);
  
  args[0] = curdir;
  args[1] = argc;
  i = 0;
  j = 0;
  for (p = 1; p < ARGS_SECTOR && i < argc; ++p) {
    args[p] = argv[i][j];
    if (argv[i][j] == '\0') {
      ++i;
      j = 0;
    } else {
      ++j;
    }
  }
  writeSector(args, ARGS_SECTOR);
}

void getCurdir (char *curdir) {
  char args[SECTOR_SIZE];
  readSector(args, ARGS_SECTOR);
  
  *curdir = args[0];
}

void getArgc (char *argc) {
  char args[SECTOR_SIZE];
  readSector(args, ARGS_SECTOR);
  *argc = args[1];
}

void getArgv (char index, char *argv) {
  char args[SECTOR_SIZE];
  int i, j, p;
  readSector(args, ARGS_SECTOR);
  i = 0;
  j = 0;
  for (p = 1; p < ARGS_SECTOR; ++p) {
    if (i == index) {
      argv[j] = args[p];
      ++j;
    }
    
    if (args[p] == '\0') {
      if (i == index) {
        break;
      } else {
        ++i;
      }
    }
  }
}

