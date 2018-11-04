#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include<pthread.h>
#include<fcntl.h>

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)


sem_t sem;
pthread_mutex_t lock;
int fd;

volatile unsigned int meter_value=0;
unsigned int flag = 0;
char buf[64];

void *usb_read_function(void *arg)
{
	while(1){
		sem_wait(&sem);
		if(flag == 1){
			for(int i = 0; i<10000; i++)
			{
				for(int j = 0; j<10000; j++);
			}
			sprintf(buf,"Meter reading: %u\n",meter_value);
			printf("%s",buf);
			write(fd,buf,64);
			close(fd);
			flag = 0;
		}
	}
}
	
void *meter_function(void *arg)
{
	while(1)
		{
			meter_value++;
			for(int i = 0; i<10000; i++)
			{
				for(int j = 0; j<10000; j++);
			}
			if(flag == 0){
				if((fd = open("/dev/custom0",O_RDWR))){
					sem_post(&sem);
					flag = 1;
				}
			}
		}


}		
	
int main(){
	pthread_t usb_read,usb_meter;
	int ret;
	void *res;
	if(sem_init(&sem,0,0) == -1){
		handle_error("semaphore initialization failed.\n");
	}	
	//if (sem_init(&sem,0,5)==-1)
	//{handle_error("sem_init");}
	ret = pthread_create(&usb_meter,NULL,meter_function,NULL); 
	if(ret != 0)
	{
		handle_error("usb_meter_function.\n");
	}
	ret = pthread_create(&usb_read,NULL,usb_read_function,NULL);
	if(ret != 0)
	{
		handle_error("usb_read_function.\n");
	}
	pthread_join(usb_read,NULL);
	pthread_join(usb_meter,NULL);
}
