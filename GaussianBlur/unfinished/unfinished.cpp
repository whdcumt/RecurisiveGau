// unfinished.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<opencv2/opencv.hpp>
#include<time.h>
#include <windows.h>
using namespace std;
using namespace cv;
/****************************************
*    src       : 原始图像数据                *
*    dst       : 模糊后图像数据            *
*    width     : 图像宽                    *
*    height    : 图像高                    *
*    sigma     : 高斯参数                    *
*    chan      : 图像通道数                *
*****************************************/
void IMG_GaussBlur(unsigned char* src, unsigned char*& dst, int width, int height, float sigma, int chan)
{
	int    i            = 0;
	int    row            = 0;
	int    col            = 0;
	int    pos            = 0;
	int    channel        = 0;
	int    n            = 0;
	int    bufsize        = 0;        
	int size        = 0;
	int rowstride    = 0;
	int itemp0        = 0;
	int itemp1        = 0;    
	float temp      = 0;
	int    channelsize = width*height;

	if (width>height)
	{
		bufsize = width;
	}
	else
	{
		bufsize = height;
	}    

	float* w1    = (float *) malloc (bufsize * sizeof (float));
	float *w2    = (float *) malloc (bufsize * sizeof (float));
	float *in    = (float *) malloc (channelsize * sizeof (float));
	float *out    = (float *) malloc (channelsize * sizeof (float));

	//----------------计算高斯核---------------------------------------//
	float  q    = 0; 
	float  q2, q3;    
	double b0;
	double b1;
	double b2;
	double b3;
	double B    = 0;
	int    N    = 3;

	if (sigma >= 2.5)
	{
		q = 0.98711 * sigma - 0.96330;
	}
	else if ((sigma >= 0.5) && (sigma < 2.5))
	{
		q = 3.97156 - 4.14554 * (float) sqrt ((double) 1 - 0.26891 * sigma);
	}
	else
	{
		q = 0.1147705018520355224609375;
	}

	q2 = q * q;
	q3 = q * q2;
	b0 = (1.57825+(2.44413*q)+(1.4281 *q2)+(0.422205*q3));
	b1 = (        (2.44413*q)+(2.85619*q2)+(1.26661 *q3));
	b2 = (                   -((1.4281*q2)+(1.26661 *q3)));
	b3 = (                                 (0.422205*q3));
	B = 1.0-((b1+b2+b3)/b0);

	//加速方法 减少循环多次/b0
	b1 /= b0;
	b2 /= b0;
	b3 /= b0;
	//----------------计算高斯核结束---------------------------------------//

	// 处理图像的多个通道
	for (channel = 0; channel < chan; channel++)
	{
		// 获取一个通道的所有像素值
		for (i = 0, pos = channel; i < channelsize ; i++, pos += chan)
		{
			/* 0-255 => 1-256 */
			in[i] = (float)(src[pos] + 1.0);
		}

		//纵向处理
		for (row=0 ;row < height; row++)
		{
			pos =  row * width;            
			size        = width;
			rowstride    = 1;    
			bufsize        = size;
			size        -= 1;

			//temp =  (in + pos)[0];    // 以每一行的第一个像素数据，复制3份，开始前向滤波（最后去掉前面的3个元素）
			//w1[0] =  (in + pos)[0];
			//w1[1] = (1-b1)*(in + pos)[1]+b1*w1[0];
			//w1[2] =  (1-b1-b2)*(in + pos)[2]+b1*w1[1] + b2*w1[0];

			//w1[0] =  (in + pos)[0];  // 左边产生噪声，右边有3像素黑色不变
			//w1[1] = (0.7)*(in + pos)[1]+0.3*w1[0];
			//w1[2] =  (0.5)*(in + pos)[2]+0.3*w1[1] + 0.2*w1[0];

			temp =  (in + pos)[0]; 
			w1[0] = temp;  // ！！！计算需要相同初值（否则噪声），但是赋值的时候不能用相同的（否则像素重复或缺失）！！！
			w1[1] =temp;
			w1[2] =temp;



			for (  n=3; n <= size ; n++)
			{
				w1[n] = (float)(B*(in + pos)[n*rowstride] +    ((b1*w1[n-1] +     b2*w1[n-2] + b3*w1[n-3] )));

			}
			w1[0] =  (in + pos)[0];  // 左边产生噪声（使用不同初始值时产生），右边有3像素黑色不变（out最后3个像素未设定）
			w1[1] =(in + pos)[1];
			w1[2] =  (in + pos)[2];

			//temp =  w1[size+3]; // 以前向滤波的最后三个数据，复制3份，开始后向滤波（最后去掉后面的3个元素）
			//(out + pos)[size]= w2[size]= (float)w1[size];  // 比较好
			//(out + pos)[size-1]=w2[size-1]=(float)((1-b1)* w1[size-1]+b1*w2[size]) ;
			//(out + pos)[size-2]=w2[size-2]= (float)((1-b1-b2)*w1[size-2]+b1*w2[size-1] +    b2*w2[size]);

			//(out + pos)[size]= w2[size]= (float)w1[size]; // 产生色块
			//(out + pos)[size-1]=w2[size-1]=(float)(0.7*w1[size-1]+0.3*w1[size]) ;
			//(out + pos)[size-2]=w2[size-2]= (float)((0.5)*w1[size-2]+0.3*w2[size-1] +    0.2*w2[size]);

			(out + pos)[size]= w2[size]= (float)w1[size]; // 效果比较好
			(out + pos)[size-1]=w2[size-1]=(float)(1.0*w1[size-1]) ;
			(out + pos)[size-2]=w2[size-2]= (float)((1.0)*w1[size-2]);
			w2[size]= (float)w1[size-2];
			w2[size-1]= (float)w1[size-2];
			w2[size-2]= (float)w1[size-2];
			//(out + pos)[size]= w2[size]= (float)w1[size];
			//(out + pos)[size-1]=w2[size-1]=(float)(1.0*w1[size]) ;
			//(out + pos)[size-2]=w2[size-2]= (float)((1.0)*w1[size]);

			//w2[size]= 255;
			//w2[size-1]=255;
			//w2[size-2]= 255;
			//(out + pos)[size]= 255;
			//(out + pos)[size-1]=255;
			//(out + pos)[size-2]= 255;

			for (n = size-3; n>= 0; n--) // 不该是size-1?
			{
				(out + pos)[n * rowstride] = w2[n] = (float)(B*w1[n] +    ((b1*w2[n+1] +    b2*w2[n+2] + b3*w2[n+3] ))); // 得到一行的输出

			}    

			//for (n = size; n>= 0; n--) // 不该是size-1?
			//{
			//	(out + pos)[n * rowstride] = w1[n]; // 得到一行的输出

			//}    

		}    


		//横向处理
		for (col=0; col < width; col++)  // wbp 在纵向处理的基础上继续横向处理
		{                
			size        = height;
			rowstride    = width;    
			bufsize        = size;
			size        -= 1;

			temp  = (out + col)[0*rowstride];  // wbp 第col列的第一个数据，复制3份，开始前向滤波
			w1[0] = temp;
			w1[1] = temp;
			w1[2] = temp;
			for ( n=3; n <= size ; n++)
			{
				w1[n] = (float)(B*(out + col)[n*rowstride] + ((b1*w1[n-1] +    b2*w1[n-2] + b3*w1[n-3] )));

			}
			w1[0] =  (out + col)[0];
			w1[1] =  (out + col)[rowstride];
			w1[2] =  (out + col)[2*rowstride];


			temp        = w1[size];
			w2[size]    = temp;
			w2[size-1]    = temp;
			w2[size-2]    = temp;
			(in + col)[size * rowstride]=w1[size];
			(in + col)[(size-1) * rowstride]=w1[size-1];
            (in + col)[(size-2) * rowstride]=w1[size-2];

			for (n = size-3; n >= 0; n--)
			{
				(in + col)[n * rowstride] =w2[n]= (float)(B*w1[n] +    ((b1*w2[n+1] +     b2*w2[n+2] + b3*w2[n+3] )));

			}                
		}

		//拷贝结果到函数输出指针 改用下面修正偏移方法拷贝数据
		/*for (i = 0, pos = channel; i < channelsize-(width+height)*3 ; i++, pos += chan)
		{            
		dst[pos] = in[i]-1;
		}*/


		//修正偏移的拷贝方法, 但是修正后图像右边及下边会丢失数据？？？// wbp 并且图像左边及上边多出数据
		for(int y=0; y<height; y++)
		{
			itemp0 = y*width*chan;
			itemp1 = (y)*width;                                // +3  数据没丢失，但是是拷贝而不是原数据
			for (int x=0; x<width; x++)
			{            
				dst[itemp0+x*chan + channel]=in[itemp1+x]-1;    // +3
			}
		}         
	}

	free (w1);
	free (w2);
	free (in);
	free (out);
}
int main(int argc, char* argv[])
{
	Mat src = imread("D:\\传统高斯滤波\\GaussianBlur\\GaussianBlur\\测试图像\\10241024.jpg");
    Mat dst = imread("D:\\传统高斯滤波\\GaussianBlur\\GaussianBlur\\测试图像\\10241024.jpg");
	LARGE_INTEGER m_nFreq;
    LARGE_INTEGER m_nBeginTime;
    LARGE_INTEGER nEndTime;
    QueryPerformanceFrequency(&m_nFreq); // 获取时钟周期
    QueryPerformanceCounter(&m_nBeginTime); // 获取时钟计数  
	for(int i=0;i<10;i++)
	{
	   IMG_GaussBlur(src.data, dst.data, src.cols, src.rows, 1, 3);  
	}
	 QueryPerformanceCounter(&nEndTime);
     cout << (nEndTime.QuadPart-m_nBeginTime.QuadPart)*100/m_nFreq.QuadPart << endl;
	 imshow("image",dst); 
	//此函数等待按键，按键盘任意键返回
	waitKey();
	while(1);
	return 0;
}


 