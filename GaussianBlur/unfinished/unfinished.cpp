// unfinished.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include<opencv2/opencv.hpp>
#include<time.h>
#include <windows.h>
using namespace std;
using namespace cv;
/****************************************
*    src       : ԭʼͼ������                *
*    dst       : ģ����ͼ������            *
*    width     : ͼ���                    *
*    height    : ͼ���                    *
*    sigma     : ��˹����                    *
*    chan      : ͼ��ͨ����                *
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

	//----------------�����˹��---------------------------------------//
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

	//���ٷ��� ����ѭ�����/b0
	b1 /= b0;
	b2 /= b0;
	b3 /= b0;
	//----------------�����˹�˽���---------------------------------------//

	// ����ͼ��Ķ��ͨ��
	for (channel = 0; channel < chan; channel++)
	{
		// ��ȡһ��ͨ������������ֵ
		for (i = 0, pos = channel; i < channelsize ; i++, pos += chan)
		{
			/* 0-255 => 1-256 */
			in[i] = (float)(src[pos] + 1.0);
		}

		//������
		for (row=0 ;row < height; row++)
		{
			pos =  row * width;            
			size        = width;
			rowstride    = 1;    
			bufsize        = size;
			size        -= 1;

			//temp =  (in + pos)[0];    // ��ÿһ�еĵ�һ���������ݣ�����3�ݣ���ʼǰ���˲������ȥ��ǰ���3��Ԫ�أ�
			//w1[0] =  (in + pos)[0];
			//w1[1] = (1-b1)*(in + pos)[1]+b1*w1[0];
			//w1[2] =  (1-b1-b2)*(in + pos)[2]+b1*w1[1] + b2*w1[0];

			//w1[0] =  (in + pos)[0];  // ��߲����������ұ���3���غ�ɫ����
			//w1[1] = (0.7)*(in + pos)[1]+0.3*w1[0];
			//w1[2] =  (0.5)*(in + pos)[2]+0.3*w1[1] + 0.2*w1[0];

			temp =  (in + pos)[0]; 
			w1[0] = temp;  // ������������Ҫ��ͬ��ֵ�����������������Ǹ�ֵ��ʱ��������ͬ�ģ����������ظ���ȱʧ��������
			w1[1] =temp;
			w1[2] =temp;



			for (  n=3; n <= size ; n++)
			{
				w1[n] = (float)(B*(in + pos)[n*rowstride] +    ((b1*w1[n-1] +     b2*w1[n-2] + b3*w1[n-3] )));

			}
			w1[0] =  (in + pos)[0];  // ��߲���������ʹ�ò�ͬ��ʼֵʱ���������ұ���3���غ�ɫ���䣨out���3������δ�趨��
			w1[1] =(in + pos)[1];
			w1[2] =  (in + pos)[2];

			//temp =  w1[size+3]; // ��ǰ���˲�������������ݣ�����3�ݣ���ʼ�����˲������ȥ�������3��Ԫ�أ�
			//(out + pos)[size]= w2[size]= (float)w1[size];  // �ȽϺ�
			//(out + pos)[size-1]=w2[size-1]=(float)((1-b1)* w1[size-1]+b1*w2[size]) ;
			//(out + pos)[size-2]=w2[size-2]= (float)((1-b1-b2)*w1[size-2]+b1*w2[size-1] +    b2*w2[size]);

			//(out + pos)[size]= w2[size]= (float)w1[size]; // ����ɫ��
			//(out + pos)[size-1]=w2[size-1]=(float)(0.7*w1[size-1]+0.3*w1[size]) ;
			//(out + pos)[size-2]=w2[size-2]= (float)((0.5)*w1[size-2]+0.3*w2[size-1] +    0.2*w2[size]);

			(out + pos)[size]= w2[size]= (float)w1[size]; // Ч���ȽϺ�
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

			for (n = size-3; n>= 0; n--) // ������size-1?
			{
				(out + pos)[n * rowstride] = w2[n] = (float)(B*w1[n] +    ((b1*w2[n+1] +    b2*w2[n+2] + b3*w2[n+3] ))); // �õ�һ�е����

			}    

			//for (n = size; n>= 0; n--) // ������size-1?
			//{
			//	(out + pos)[n * rowstride] = w1[n]; // �õ�һ�е����

			//}    

		}    


		//������
		for (col=0; col < width; col++)  // wbp ��������Ļ����ϼ���������
		{                
			size        = height;
			rowstride    = width;    
			bufsize        = size;
			size        -= 1;

			temp  = (out + col)[0*rowstride];  // wbp ��col�еĵ�һ�����ݣ�����3�ݣ���ʼǰ���˲�
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

		//����������������ָ�� ������������ƫ�Ʒ�����������
		/*for (i = 0, pos = channel; i < channelsize-(width+height)*3 ; i++, pos += chan)
		{            
		dst[pos] = in[i]-1;
		}*/


		//����ƫ�ƵĿ�������, ����������ͼ���ұ߼��±߻ᶪʧ���ݣ�����// wbp ����ͼ����߼��ϱ߶������
		for(int y=0; y<height; y++)
		{
			itemp0 = y*width*chan;
			itemp1 = (y)*width;                                // +3  ����û��ʧ�������ǿ���������ԭ����
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
	Mat src = imread("D:\\��ͳ��˹�˲�\\GaussianBlur\\GaussianBlur\\����ͼ��\\10241024.jpg");
    Mat dst = imread("D:\\��ͳ��˹�˲�\\GaussianBlur\\GaussianBlur\\����ͼ��\\10241024.jpg");
	LARGE_INTEGER m_nFreq;
    LARGE_INTEGER m_nBeginTime;
    LARGE_INTEGER nEndTime;
    QueryPerformanceFrequency(&m_nFreq); // ��ȡʱ������
    QueryPerformanceCounter(&m_nBeginTime); // ��ȡʱ�Ӽ���  
	for(int i=0;i<10;i++)
	{
	   IMG_GaussBlur(src.data, dst.data, src.cols, src.rows, 1, 3);  
	}
	 QueryPerformanceCounter(&nEndTime);
     cout << (nEndTime.QuadPart-m_nBeginTime.QuadPart)*100/m_nFreq.QuadPart << endl;
	 imshow("image",dst); 
	//�˺����ȴ����������������������
	waitKey();
	while(1);
	return 0;
}


 