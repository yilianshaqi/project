/************************************************************************
    > File Name: test_main.c
    > Author: huang yang
    > Mail: huangyangwork@sina.com 
    > Created Time: 2016年12月09日 星期五 11时20分21秒
 ************************************************************************/

#include<stdio.h>
#include <unistd.h>
#include"camera.h"
void *init_lcd(void *args);
int main()
{
	start("/dev/video0","/tmp/pict");
//	START("/home/hy");
	pthread_t lcd_pthread;
	sleep(2);
	pthread_create(&lcd_pthread,NULL,init_lcd,NULL);
	printf("睡   完，stop调用\n");
	sleep(5);
//	int i,j;
//	while(1);
//	for( i =0;i<10000000;i++);
//	{
	//	for(j=0;j<10000000;i++);
	
//	}
//	stop();
	sleep(200);
	return 0;
}

