#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(){
	struct input_event fey_dev;
	
	int fd_key = open("/dev/input/event5",O_RDWR);
	
	if(fd_key < 0)
	{
		perror("open /dev/input/event5");
		
		return -1;
		
	}
	
	while(1)
	{
		read(fd_key,&key_dev,sizeof key_val_cur);
		
		if(key_dev.type==EV_KEY){
			if(key_dev.code==KEY_ENTER){
				if(key_dev.value)
					printf("KEY_ENTER Down\n");
				else
					printf("KEY_ENTER Up\n");	
			}
			
			if(key_dev.code == KEY_Z){
				if(key_dev.value)
					printf("KEY_Z Down\n");
				else
					printf("KEY_Z Up\n");
			}		

			if(key_dev.code == KEY_X){
				if(key_dev.value)
					printf("KEY_X Down\n");
				else
					printf("KEY_X Up\n");
			}		
			
			if(key_dev.code == KEY_RIGHT){
				if(key_dev.value)
					printf("KEY_RIGHT Down\n");
				else
					printf("KEY_RIGHT Up\n");
			}	
			
		}
		
		printf("main is running ...\n");
	}
	
	close(fd_key);
	
	return 0;
	
}

