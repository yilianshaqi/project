/*************************************************************************
    > File Name: demo_init.c
    > Author: huang yang
    > Mail: huangyangwork@sina.com 
    > Created Time: 2016年12月07日 星期三 19时13分12秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<string.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <linux/fb.h>
 #include <unistd.h>
#include <asm/types.h>
#include<errno.h>
#include <unistd.h>
#include<sys/mman.h>
int  BUF_SIZE= 10;
int img_width = 640;
int img_height = 480;
int main(){
	int fd = open("/dev/video2",O_RDWR);
	if(fd < 0)
	{
		printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
		return -1;
	}
	//查看设备支持格式
	struct v4l2_fmtdesc getformat;
	bzero(&getformat,sizeof(getformat));
	getformat.index=0;
	getformat.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(fd,VIDIOC_ENUM_FMT,&getformat))
	{
		perror("VIDIOC_ENUM_FMT:\n");
		printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
		return -1;
	}
	printf("pixelformat=%c%c%c%c,description=%s\n",getformat.pixelformat & 0xff,(getformat.pixelformat>>8)&0xff,(getformat.pixelformat>>16)&0xff,(getformat.pixelformat>>24)&0xff,getformat.description);
	//查看设备能力
	struct v4l2_capability  cap;
	bzero(&cap,sizeof(cap));
	if(ioctl(fd,VIDIOC_QUERYCAP,&cap))
	{
		printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
		return -1;
	}
	printf("device_capability:%x\n",cap.capabilities);
	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
		return -1;
	}
/*	没有读到任何东西也不阻塞
 *	char buf=0;
	read(fd,&buf,1);
	printf("read:%c\n",buf);
	return 0;  */  
		//剪裁缩放设置
/*	struct v4l2_cropcap cscap;
	bzero(&cscap,sizeof(cscap));
	cscap.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(fd,VIDIOC_CROPCAP,&cscap))
	{
		printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
		return -1;
	}
	struct v4l2_crop crop;
	bzero(&crop,sizeof(crop));
	crop.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c=cscap.defrect;
//	crop.c=cscap.bounds;
	if(ioctl(fd,VIDIOC_S_CROP,&crop))
	{
		perror("crop:\n");
		printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
		return -1;
	}
*/
  // 剪裁不支持



	//设置视频支持的格式参数
	struct v4l2_format format;
	bzero(&format,sizeof(format));
	format.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width=img_width;
	format.fmt.pix.height=img_height;
	format.fmt.pix.pixelformat=V4L2_PIX_FMT_YUYV;
	format.fmt.pix.field=V4L2_FIELD_INTERLACED;
	if(ioctl(fd,VIDIOC_S_FMT,&format))
	{
		printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
		return -1;
	}
	printf("格式参数设置完毕\n");

	//参数设置后查看
	bzero(&format,sizeof(format));
	format.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(fd,VIDIOC_G_FMT,&format))
	{
		printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
		return -1;
	}
	printf("width:%d,height:%d\n",format.fmt.pix.width,format.fmt.pix.height);
	printf("spaceperlien:%d,field:%d\n",format.fmt.pix.bytesperline,format.fmt.pix.field & V4L2_FIELD_TOP);
	printf("format.fmt.pix.pixelformat :%X,%X\n",format.fmt.pix.pixelformat,format.fmt.pix.pixelformat & getformat.pixelformat);

	//设置帧率
	struct v4l2_streamparm sparm;
	bzero(&sparm,sizeof(sparm));
	sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(fd,VIDIOC_G_PARM,&sparm))
	{
		printf("查看帧率参数失败:%s\n",strerror(errno));
	}
	printf("capability :%d,numerator:%d,denominator:%d\n",sparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME,sparm.parm.capture.timeperframe.numerator,sparm.parm.capture.timeperframe.denominator);

	bzero(&sparm,sizeof(sparm));
	sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	sparm.parm.capture.capability=V4L2_CAP_TIMEPERFRAME;
	sparm.parm.capture.timeperframe.numerator = 1;
	sparm.parm.capture.timeperframe.denominator = 10;
	if(ioctl(fd,VIDIOC_S_PARM,&sparm))
	{
		printf("set parm error\n");
	}
	printf("capability :%d,numerator:%d,denominator:%d\n",sparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME,sparm.parm.capture.timeperframe.numerator,sparm.parm.capture.timeperframe.denominator);


	//申请内存缓冲区

	struct v4l2_requestbuffers  buffers;
	bzero(&buffers,sizeof(buffers));
	buffers.count = BUF_SIZE;
	buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffers.memory = V4L2_MEMORY_MMAP; 
	if(ioctl(fd,VIDIOC_REQBUFS,&buffers))
	{
		printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
		return -1;
	//申请的内存不够用时退出时要释放申请
	}
	BUF_SIZE = buffers.count;
	printf("buf_count:%d\n",buffers.count);
	
	//获得申请的缓冲内存去映射到用户空间
	struct BUFFER {
		unsigned int length;
		void *start;
	};
	struct BUFFER *buf=NULL;
	buf = calloc(BUF_SIZE,sizeof(struct BUFFER));
	if(NULL == buf)
	{
		printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
		return -1;
	}
	printf("buf:%p\n",buf);
	bzero(buf,BUF_SIZE * sizeof(struct BUFFER));
	printf("buf[0].start=%p\n",buf[0].start);

	struct v4l2_buffer getbuf;
	for(int i= 0;i<BUF_SIZE;i++)
	{
		bzero(&getbuf,sizeof(getbuf));
		getbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		getbuf.memory = V4L2_MEMORY_MMAP;
		getbuf.index = i;
		if(ioctl(fd,VIDIOC_QUERYBUF,&getbuf))
		{
		printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
		return -1;
		}

		buf[i].length = getbuf.length;
		buf[i].start = mmap(NULL,getbuf.length,PROT_READ | PROT_WRITE,MAP_SHARED,fd,getbuf.m.offset);
		if(NULL == buf[i].start)
		{
			printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
			return -1;
			
		}
		printf("start%d:%p\n",i,buf[i].start);
		
	}	
	//添加缓冲队列
	struct 	v4l2_buffer addbuf;
	for(int i=0;i<BUF_SIZE;i++)
	{
		bzero(&addbuf,sizeof(addbuf));
		addbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		addbuf.memory = V4L2_MEMORY_MMAP;
		addbuf.index = i;
		if(ioctl(fd,VIDIOC_QBUF,&addbuf))
		{
			printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
			return -1;
		}
	}
	//开启视频采集
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(fd,VIDIOC_STREAMON,&type))
	{
			printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
			return -1;
	}
	//select 读取采集的视频数据
	struct v4l2_buffer getdata;
	int num = 100;
	while(num--)
	{


		bzero(&getdata,sizeof(getdata));
		fd_set set;
		FD_ZERO(&set);
		FD_SET(fd,&set);
		struct timeval tv;
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		printf("before select\n");
		int ret = select(fd+1,&set,NULL,NULL,&tv);
		printf("after select\n");
		if(ret > 0)
		{
			//获取采集满的缓冲区
			bzero(&getdata,sizeof(getdata));
			getdata.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			getdata.memory = V4L2_MEMORY_MMAP;
			if(ioctl(fd,VIDIOC_DQBUF,&getdata))
			{
				printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
				return -1;
			}
			int fs = open("picture",O_RDWR | O_CREAT,0666);
			if(fs<0)
			{
				printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
				return -1;
			}
			int rest = write(fs,buf[getdata.index].start,buf[getdata.index].length);
			if(rest <0)
			{
				printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
				return -1;
			}
			close(fs);  
			printf("index:%d\n",getdata.index);
			/*		FILE *fs = fopen("picture","a+");
					if(fs==NULL)
					{
					printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
					return -1;
					}
					int rest = fwrite(buf[getdata.index].start,buf[getdata.index].length,1,fs);
					if(rest<=0)
					{
					printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
					return -1;

					}
					fclose(fs);   */
			//将缓冲区才添加回等待队列

			bzero(&addbuf,sizeof(addbuf));
			addbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			addbuf.memory = V4L2_MEMORY_MMAP;
			addbuf.index = getdata.index;
			if(ioctl(fd,VIDIOC_QBUF,&addbuf))
			{
				printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
				return -1;
			}
		}
	}
	//关闭视频采集数据流	
	
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(fd,VIDIOC_STREAMOFF,&type))
	{
			printf("filename:%s,func:%s,line:%d\n",__FILE__,__func__,__LINE__);
			return -1;
	}
	//关闭映射
	for(int i= 0;i<BUF_SIZE;i++)
	{
		munmap(buf[i].start,buf[i].length);
	}
	//释放calloc申请的buf
		free(buf);
	//关闭设备
	close(fd);

	return 0;
}
