#define SECTOR_SIZE 512
#define DIRS_SECTOR 0x101
#define SUCCESS 0
#define NOT_FOUND -1

int strncmp(char *a, char *b, int len);
void handleArg(char *input);

void main(){
	char curDir, useDir;
	char input[512];
	char dir[SECTOR_SIZE];
	int success=1, offset;
	int se[4], i;
	interrupt(0x21, 0x21, &curDir, 0, 0);
		// read input
		interrupt(0x21, 0x00, "$ ", 0, 0);
		interrupt(0x21, 0x01, input, 0, 0);
		interrupt(0x21, 0x00, "\r\n", 0, 0);

		if(strncmp(input,"cd", 2)){
			useDir = curDir;
			offset = 2;
			while(input[offset]==' ')
				offset++;
			if(input[offset]==0){
				interrupt(0x21, 0x20, 0xFF, 0, 0);
			}
			else if(input[2] == ' '){
				// interrupt(0x21, 0x00, input+offset, 0, 0);
				// interrupt(0x21, 0x00, "\r\n", 0, 0);
				interrupt(0x21, 0x30, input+offset, se, &useDir);
				if(se[2] == NOT_FOUND){
					interrupt(0x21, 0x00, "ERROR! Directory Not Found.\r\n", 0, 0);
				}
				else{
					interrupt(0x21, 0x02, dir, DIRS_SECTOR, 0);
					interrupt(0x21, useDir<<8|0x31, dir, input+offset, se);
					if(se[2] == NOT_FOUND){
						interrupt(0x21, 0x00, "ERROR! Directory Not Found.\r\n", 0, 0);
					}
					else{
						curDir = (char)se[2];
						interrupt(0x21, 0x20, curDir, 0, 0);
					}
				}
			}
		}
		else if(strncmp(input,"exit", 4)){
			//break;
		}
		else if(input[0]!=0){
			// Prepare the argument
			useDir = 0xFF;
			offset = 0;
			if(strncmp(input, "./", 2)){
				useDir = curDir;
				offset = 2;
			}
			handleArg(input+offset, curDir);
			// Run the program
			for(i=offset;input[i]!=' ' && input[i]!='\0';i++);
			input[i] = '\0';
			interrupt(0x21, 0xFF<<8|0x6, input, 0x2000, &success);
			if(success!=SUCCESS){
				if(success == NOT_FOUND)
					interrupt(0x21, 0x00, "ERROR! Program Not Found.\r\n", 0, 0);
				else
					interrupt(0x21, 0x00, "ERROR OCCURRED!!\r\n", 0, 0);
			}
		}

	interrupt(0x21, 0x07, &success, 0, 0);
}


void handleArg(char *input, curDir){
	int i =0, j=0;
	int argc = 0;
	char *argv[16];
	char dummy[16][128];
	while(input[i]!=' ' && input[i]!='\0')
		i++;
	if(input[i]!='\0'){
		while(1){
			if(input[i]==' '||input[i]=='\0'){
				if(j>0){
					dummy[argc][j] = '\0';
					argv[argc] = dummy[argc];
					argc++;
					j = 0;
					if(input[i]=='\0')
						break;
				}
			}
			else{
				dummy[argc][j] = input[i];
				j++;
			}
			i++;
		}
	}
	interrupt(0x21, 0x20, curDir, argc, argv);

}

int strncmp(char *a, char *b, int len){
	int i;
	for(i=0;i<len;i++){
		if(a[i]=='\0' || b[i] == '\0'){
			return 0;
		}
		if(a[i]!=b[i]){
			return 0;
		}
	}
	return 1;
}
