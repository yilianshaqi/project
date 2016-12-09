/*************************************************************************
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
	start("/dev/video1","/home/hy/project/picture/pict");
//	START("/home/hy");
	sleep(1);
	stop();
	sleep(1);
	return 0;
}

