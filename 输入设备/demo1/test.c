#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>


int main(int argc,char **argv)
{

	struct input_event key_dev;

	
	int fd_key = open("/dev/input/event5",O_RDWR);
	
	if(fd_key < 0)
	{
		perror("open /dev/input/event5");
		
		return -1;
		
	}
	

	while(1)
	{
		read(fd_key,&key_dev,sizeof key_dev);
		
		if(key_dev.type == EV_KEY)
		{
			if(key_dev.code == KEY_ENTER)
			{
				if(key_dev.value)
					printf("KEY_ENTER Down\n");
				else
					printf("KEY_ENTER Up\n");
			}
			
			
		}
		
		printf("main is running ...\n");
	}

	close(fd_key);

	return 0;
	
}