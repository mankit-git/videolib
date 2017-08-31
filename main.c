
#include "video.h"

int main(int argc, char const *argv[])
{
	// 打开LCD设备
	int lcd = open("/dev/fb0", O_RDWR);

	// 获取LCD显示器的设备参数
	struct fb_var_screeninfo lcdinfo;
	ioctl(lcd, FBIOGET_VSCREENINFO, &lcdinfo);

	unsigned char *fbmemy = lcd_mem(lcd, &lcdinfo);

	// 打开摄像头设备文件
	int cam_fd = open("/dev/video7",O_RDWR); //video7这个路径不是一定的，根据情况而定

	int nbuf = camera(cam_fd, lcdinfo);
	
	unsigned char *start[nbuf];
	int length[nbuf];
	video_catchdata(nbuf, cam_fd, start, length);

	struct v4l2_buffer v4lbuf;
	bzero(&v4lbuf, sizeof(v4lbuf));
	v4lbuf.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4lbuf.memory= V4L2_MEMORY_MMAP;

	unsigned char *rgb_buf;
	struct image_info *image_info = calloc(1, sizeof(struct image_info));
	int i = 0;
	while(1)
	{
		// 从队列中取出填满数据的缓存
		v4lbuf.index = i%nbuf;
		ioctl(cam_fd , VIDIOC_DQBUF, &v4lbuf); // VIDIOC_DQBUF在摄像头没数据的时候会阻塞
		rgb_buf = shooting(start[i%nbuf], length[i%nbuf], image_info);
		write_lcd(fbmemy, &lcdinfo, rgb_buf, image_info, 0, 0);

	 	// 将已经读取过数据的缓存块重新置入队列中 
		v4lbuf.index = i%nbuf;
		ioctl(cam_fd , VIDIOC_QBUF, &v4lbuf);

		i++;
	}

	return 0;
}