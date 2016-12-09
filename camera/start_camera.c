/*************************************************************************
    > File Name: start_camera.c
    > Author: huang yang
    > Mail: huangyangwork@sina.com 
    > Created Time: 2016年12月09日 星期五 11时05分02秒
 ************************************************************************/
#include"camera_pthread.h"
void *start_camera(void *argv)
{
	printf("start pthread\n");
	camera_log_error = fopen("camera_log","w+");
	if(NULL == camera_log_error)
	{
		pthread_exit(0);
	}
	if(NULL == argv)
	{
		log_exit("argv");
	}
	char *devname = NULL;
	devname = strtok(argv,"-");
	filename = strtok(NULL,"-");
	if(NULL == devname || NULL == filename)
	{
		log_exit("strtok argv");
	}
//	printf("devname :%s\n",devname);
//	printf("filename : %s\n",filename);
	
	fd = 0;
//	struct BUFFER *buf=NULL;
	picture_count =1;
	buf_count = 20;
	condition = 1;
	//停止信号处理监听
	signal(SIGUSR1,stop_handler);
	//初始化函数
	printf("file:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
	init_device(devname);

	//申请缓存
	request_queue();
	//初始化缓存映射保存区
	init_buf(&buf);
	//缓冲区映射
	init_mmap();
	//加入缓冲队列
	add_queue();
	//开启视频采集
	start_capture();
	//定时信号处理
	signal(SIGALRM,loop_handler);
	ualarm(1,RATE);
	while(1)
	{
		sleep(100);
	}
	printf("get data\n");
	return 0;
}
void log_exit(char *error)
{
	fprintf(camera_log_error,"%s error:\n",error);
	pthread_exit(NULL);
}
void ckioctl(int fd,int macro,void *st)
{
	if(ioctl(fd,macro,st))
	{
		char buf[5]="";
		sprintf(buf,"%0x",macro);
		log_exit(buf);
	}
	return ;
}
int init_device(char *devname)
{
	//打开设备
	open_device(devname);
	//检查设备能力
	query_cap();
	//设置图片大小
	set_size();

}
//打开设备文件
void open_device(char *devname)
{
	fd = open(devname,O_RDWR);
	if(fd<0)
	{
		log_exit("open device");
	}
}
//检查设备能力：查看是否具有V4L2_BUF_TYPE_VIDEO_CAPTURE能力和支持读写能力
void query_cap(void)
{
	struct v4l2_capability cap;
	bzero(&cap,sizeof(cap));
	ckioctl(fd,VIDIOC_QUERYCAP,&cap);
	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		log_exit("query_cap capture");
	}
	if(!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		log_exit("query_cap streaming");
	}
}
//设置采集图片大小
void set_size(void )
{
	struct v4l2_format format;
	bzero(&format,sizeof(format));
	format.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width=IMAGE_WIDTH;
	format.fmt.pix.height=IMAGE_HEIGHT;
	format.fmt.pix.pixelformat=V4L2_PIX_FMT_YUYV;
	format.fmt.pix.field=V4L2_FIELD_INTERLACED;
	ckioctl(fd,VIDIOC_S_FMT,&format);
	printf("file:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
}
//申请内存缓冲区
void request_queue(void)
{
	struct v4l2_requestbuffers  buffers;
	bzero(&buffers,sizeof(buffers));
	buffers.count = buf_count;
	buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffers.memory = V4L2_MEMORY_MMAP; 
	ckioctl(fd,VIDIOC_REQBUFS,&buffers);
	buf_count = buffers.count;  //成功申请的buf数量
	if(buf_count<0)
	{
		log_exit("request buf");
	}
}
//初始化缓存映射保存区
void init_buf(struct BUFFER **buf)
{
	printf("buf_count:%d\n",buf_count);
	*buf = calloc(buf_count,sizeof(struct BUFFER));
	if(NULL ==  buf)
	{
		log_exit("calloc buf");
	}
	bzero(*buf,buf_count*sizeof(struct BUFFER));
}
//缓存映射
void init_mmap(void)
{
	struct v4l2_buffer getbuf;
	for(int i=0;i<buf_count;i++)
	{
		bzero(&getbuf,sizeof(getbuf));
		getbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		getbuf.memory = V4L2_MEMORY_MMAP;
		getbuf.index = i;
		ckioctl(fd,VIDIOC_QUERYBUF,&getbuf);
		buf[i].length = getbuf.length;
		buf[i].start = mmap(NULL,getbuf.length,PROT_READ | PROT_WRITE,MAP_SHARED,fd,getbuf.m.offset);
		if(NULL == buf[i].start)
		{
			log_exit("mmap");
		}

	}
}

void add_queue(void)
{
	struct v4l2_buffer addbuf;
	for(int i=0;i<buf_count;i++)
	{
		bzero(&addbuf,sizeof(addbuf));
		addbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		addbuf.memory = V4L2_MEMORY_MMAP;
		addbuf.index = i;
		ckioctl(fd,VIDIOC_QBUF,&addbuf);
	}
}
void start_capture(void)
{
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ckioctl(fd,VIDIOC_STREAMON,&type);
}
	//保存视频采集数据
void collect_data(void)
{
	struct v4l2_buffer getdata;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(fd,&set);
	int ret = select(fd+1,&set,NULL,NULL,NULL);
	if(ret <= 0)
	{
		log_exit("select");
	}
	//获取采集满的缓冲区
	bzero(&getdata,sizeof(getdata));
	getdata.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	getdata.memory = V4L2_MEMORY_MMAP;
	ckioctl(fd,VIDIOC_DQBUF,&getdata);

	//保存文件名设置
	char filepath[256]="";
	sprintf(filepath,"%s%d",filename,picture_count);
	printf("filepath:%s\n",filepath);
	picture_count++;
	//保存数据

	int fs = open(filepath,O_RDWR | O_CREAT,0666);
	if(fs< 0)
	{
		log_exit("open Storage file");
	}
	ret = write (fs,buf[getdata.index].start,buf[getdata.index].length);
	if(ret < 0)
	{
		log_exit("write storage file");
	}
	close(fs);

	//将缓冲区才添加回等待队列

	struct 	v4l2_buffer addbuf;
	bzero(&addbuf,sizeof(addbuf));
	addbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	addbuf.memory = V4L2_MEMORY_MMAP;
	addbuf.index = getdata.index;
	ckioctl(fd,VIDIOC_QBUF,&addbuf);
}
//停止视频数据采集
void stop_capture(void)
{
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ckioctl(fd,VIDIOC_STREAMOFF,&type);
}
//关闭视频设备
void close_device(void)
{
	for(int i= 0;i<buf_count;i++)
	{
		munmap(buf[i].start,buf[i].length);
	}
	close(fd);
}

void loop_handler(int signum)
{
	if(condition)
	{
		//获得图片
		collect_data();
		//设置定时器
	}	
	else
	{
		stop_capture();
		close_device();
		pthread_exit(NULL);
	}
}
void stop_handler(int signum)
{
	printf("get stop signal\n");
	condition = 0;
}
