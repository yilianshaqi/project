/************************************************************************
    > File Name: test_main.c
    > Author: huang yang
    > Mail: huangyangwork@sina.com 
    > Created Time: 2016年12月09日 星期五 11时20分21秒
 ************************************************************************/

#include<stdio.h>
#include <unistd.h>
#include"camera.h"
int main()
{
	start("/dev/video5","/home/hy/project/picture/pict");
//	START("/home/hy");

	printf("睡   完，stop调用\n");
	sleep(3);
	int i,j;
	for( i =0;i<10000000;i++);
//	{
	//	for(j=0;j<10000000;i++);
	
//	}
	stop();
	sleep(2);
	return 0;
}

