#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(){
	int i=0;
	unsigned char key_val;
	
	int fd_key=open("/dev/mykey",O_RDWR);
	if(fd_key<0){
		perror("open /dev/mykey");
		return -1;	
	}
	
	while(1)
	{
		read(fd_key,&key_val,sizeof key_val);
		
		if(key_val & 0x01)
			printf("K2 Down\n");
		
		printf("main is running ...\n");
	}
	
	close(fd_key);
	
	return 0;
	
}

