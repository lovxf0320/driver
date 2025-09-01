#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(){
	int i=0;
	unsigned char key_val_cur;
	unsigned char key_val_old;
	//边沿检测
	
	int fd_key=open("/dev/mykey",O_RDWR);
	if(fd_key<0){
		perror("open /dev/mykey");
		return -1;	
	}
	
	while(1)
	{
		read(fd_key,&key_val_cur,sizeof key_val_cur);
		
		if(key_val_cur & 0x01){
			printf("K2 Down\n");
		}else{
			if(key_val_old & 0x01){
				printf("K2 Up\n");
			}
		}

		if(key_val_cur & 0x02){
			printf("K3 Down\n");
		}else{
			if(key_val_old & 0x02){
				printf("K3 Up\n");
			}
		}

		if(key_val_cur & 0x03){
			printf("K4 Down\n");
		}else{
			if(key_val_old & 0x03){
				printf("K4 Up\n");
			}
		}

		if(key_val_cur & 0x04){
			printf("K6 Down\n");
		}else{
			if(key_val_old & 0x04){
				printf("K6 Up\n");
			}
		}

		if(key_val_old!=key_val_cur){
			key_val_old=key_val_cur;
		}

		printf("main is running ...\n");
	}
	
	close(fd_key);
	
	return 0;
	
}

