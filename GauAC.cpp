#define WIN32_LEAN_AND_MEAN		// trim the excess fat from Windows

/*******************************************************************
*	Program: Chapter 7 Bitmap Example 2
*	Author: Kevin Hawkins
*	Description: 递归高斯模糊  2014/4/23 22:00.
********************************************************************/

////// Defines
#define BITMAP_ID 0x4D42		// the universal bitmap ID


////// Includes
#include "StdAfx.h"
#include <windows.h>			// standard Windows app include
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gl/gl.h>				// standard OpenGL include
#include <gl/glu.h>				// OpenGL utilties
//#include <gl/glaux.h>			// OpenGL auxiliary functions
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <string>

using namespace std;
using namespace cv;
////// Global Variables
HDC g_HDC;						// global device context
bool fullScreen = false;				// true = fullscreen; false = windowed
bool keyPressed[256];				// holds true for keys that are pressed		
int width = 512;
int height = 512;
////// Bitmap Information
BITMAPINFOHEADER	bitmapInfoHeader;	// bitmap info header
unsigned char*		bitmapData;		// the bitmap data
unsigned char*	    bitmapData2;
// 高斯半径设置
int step =0;

Point2f srcTri[3];
Point2f dstTri[3];

Mat rot_mat( 2, 3, CV_32FC1 );
Mat warp_mat( 2, 3, CV_32FC1 );
Mat src, warp_dst, warp_rotate_dst;
void IMG_GaussBlur(unsigned char* src, unsigned char*& dst, int width, int height, float sigma, int chan);
// ----------------------------------------------
unsigned int listBase;				// display list base

unsigned int CreateBitmapFont(char *fontName, int fontSize)
{
	HFONT hFont;         // windows font
	unsigned int base;

	base = glGenLists(96);      // create storage for 96 characters

	if (stricmp(fontName, "symbol") == 0)
	{
		hFont = CreateFont(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, SYMBOL_CHARSET, 
			OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
			FF_DONTCARE | DEFAULT_PITCH, fontName);
	}
	else
	{
		hFont = CreateFont(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
			OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
			FF_DONTCARE | DEFAULT_PITCH, fontName);
	}

	if (!hFont)
		return 0;

	SelectObject(g_HDC, hFont);
	wglUseFontBitmaps(g_HDC, 32, 96, base);

	return base;
}

void ClearFont(unsigned int base)
{
	if (base != 0)
		glDeleteLists(base, 96);
}

void PrintString(unsigned int base, char *str)
{
	if ((base == 0) || (str == NULL))
		return;

	glPushAttrib(GL_LIST_BIT);
	glListBase(base - 32);
	glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
	glPopAttrib();
}


void CleanUp()
{
	ClearFont(listBase);
}

//----------------------------------------------
// DrawBitmap
// desc: draws the bitmap image data in bitmapImage at the location
//		 (350,300) in the window. (350,300) is the lower-left corner
//		 of the bitmap.
void DrawBitmap(long width, long height, unsigned char* bitmapImage)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	//glRasterPos2i(0,511); // 位图相对于屏幕左下角的位置
	//////
	//glPixelZoom(1.0f,-1.0f);

	//glRasterPos2i(100,50);
	glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, bitmapImage);
}


// Initialize
// desc: initializes OpenGL
void Initialize()
{
	// enable depth buffer, and backface culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Clear background to black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


	///// 加载源图像
	//src = imread( "D:\\1.jpg", 1 );

	///// 设置目标图像的大小和类型与源图像一致
	//warp_dst = Mat::zeros( src.rows, src.cols, src.type() );
	//bitmapData = src.data;

	////bitmapData = src.data;
	//// swap the R and B values to get RGB since the bitmap color format is in BGR
	//for (int imageIdx = 0; imageIdx < 512*512*3; imageIdx+=3)
	//{
	//	unsigned char tempRGB;
	//	tempRGB = bitmapData[imageIdx];
	//	bitmapData[imageIdx] = bitmapData[imageIdx + 2];
	//	bitmapData[imageIdx + 2] = tempRGB;
	//}

	//LARGE_INTEGER nFreq;
	//LARGE_INTEGER nBeginTime;
	//LARGE_INTEGER nEndTime;
	//double time1, time2,time3;
	//QueryPerformanceFrequency(&nFreq);
	//QueryPerformanceCounter(&nBeginTime); 

	bitmapData2=new unsigned char[ width*height*3];

	//QueryPerformanceCounter(&nEndTime);
	//time3=(double)(nEndTime.QuadPart-nBeginTime.QuadPart)*1000/(double)nFreq.QuadPart;


}

// Render
// desc: handles drawing of scene
void Render()
{
	// clear screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);												

	// draw the bitmap image
	DrawBitmap(512, 512, bitmapData2);

	//glFlush();
	SwapBuffers(g_HDC);			// bring backbuffer to foreground
}

// function to set the pixel format for the device context
void SetupPixelFormat(HDC hDC)
{
	int nPixelFormat;					// our pixel format index

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// size of structure
		1,								// default version
		PFD_DRAW_TO_WINDOW |			// window drawing support
		PFD_SUPPORT_OPENGL |			// OpenGL support
		PFD_DOUBLEBUFFER,				// double buffering support
		PFD_TYPE_RGBA,					// RGBA color mode
		32,								// 32 bit color mode
		0, 0, 0, 0, 0, 0,				// ignore color bits, non-palettized mode
		0,								// no alpha buffer
		0,								// ignore shift bit
		0,								// no accumulation buffer
		0, 0, 0, 0,						// ignore accumulation bits
		16,								// 16 bit z-buffer size
		0,								// no stencil buffer
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main drawing plane
		0,								// reserved
		0, 0, 0 };						// layer masks ignored

		nPixelFormat = ChoosePixelFormat(hDC, &pfd);	// choose best matching pixel format

		SetPixelFormat(hDC, nPixelFormat, &pfd);		// set pixel format to device context
}

// the Windows Procedure event handler
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HGLRC hRC;					// rendering context
	static HDC hDC;						// device context
	int width, height;					// window width and height

	switch(message)
	{
	case WM_CREATE:					// window is being created

		hDC = GetDC(hwnd);			// get current window's device context
		g_HDC = hDC;
		SetupPixelFormat(hDC);		// call our pixel format setup function

		// create rendering context and make it current
		hRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, hRC);

		return 0;
		break;

	case WM_CLOSE:					// windows is closing

		// deselect rendering context and delete it
		wglMakeCurrent(hDC, NULL);
		wglDeleteContext(hRC);

		// send WM_QUIT to message queue
		PostQuitMessage(0);

		return 0;
		break;

	case WM_SIZE:
		height = HIWORD(lParam);		// retrieve width and height
		width = LOWORD(lParam);

		if (height==0)					// don't want a divide by zero
		{
			height=1;					
		}

		glViewport(0, 0, width, height);	// reset the viewport to new dimensions
		glMatrixMode(GL_PROJECTION);		// set projection matrix current matrix
		glLoadIdentity();					// reset projection matrix

		// calculate aspect ratio of window
		//gluPerspective(54.0f,(GLfloat)width/(GLfloat)height,1.0f,1000.0f);
		glOrtho(0.0f, width - 1.0, 0.0, height - 1.0, -1.0, 1.0);

		glMatrixMode(GL_MODELVIEW);			// set modelview matrix
		glLoadIdentity();					// reset modelview matrix

		return 0;
		break;

	case WM_LBUTTONDOWN:
		++step;
		return 0;
		break;
	case WM_RBUTTONDOWN:
		step+=10;
		return 0;
		break;


	case WM_KEYDOWN:					// is a key pressed?
		keyPressed[wParam] = true;
		return 0;
		break;

	case WM_KEYUP:
		keyPressed[wParam] = false;
		return 0;
		break;

	default:
		break;
	}

	return (DefWindowProc(hwnd, message, wParam, lParam));
}

// the main windows entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX windowClass;		// window class
	HWND	   hwnd;			// window handle
	MSG		   msg;				// message
	bool	   done;			// flag saying when our app is complete
	DWORD	   dwExStyle;		// Window Extended Style
	DWORD	   dwStyle;			// Window Style
	RECT	   windowRect;



	// fill out the window class structure
	windowClass.cbSize			= sizeof(WNDCLASSEX);
	windowClass.style			= CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc		= WndProc;
	windowClass.cbClsExtra		= 0;
	windowClass.cbWndExtra		= 0;
	windowClass.hInstance		= hInstance;
	windowClass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);	// default icon
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);		// default arrow
	windowClass.hbrBackground	= NULL;								// don't need background
	windowClass.lpszMenuName	= NULL;								// no menu
	windowClass.lpszClassName	= "MyClass";
	windowClass.hIconSm			= LoadIcon(NULL, IDI_WINLOGO);		// windows logo small icon

	// register the windows class
	if (!RegisterClassEx(&windowClass))
		return 0;
	windowRect.left=(long)0;						// Set Left Value To 0
	windowRect.right=(long)width;					// Set Right Value To Requested Width
	windowRect.top=(long)0;							// Set Top Value To 0
	windowRect.bottom=(long)height;					// Set Bottom Value To Requested Height

	// class registered, so now create our window
	hwnd = CreateWindowEx(NULL,									// extended style  WS_EX_APPWINDOW | WS_EX_WINDOWEDGE
		"MyClass",							// class name
		"Bitmap Example 1: Displaying a bitmap",		// app name
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |
		WS_CLIPSIBLINGS,
		0, 0,									// x,y coordinate
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,	// width, height
NULL,									// handle to parent
		NULL,									// handle to menu
		hInstance,							// application instance
		NULL);								// no extra params

	// check if window creation failed (hwnd would equal NULL)
	if (!hwnd)
		return 0;

	ShowWindow(hwnd, SW_SHOW);			// display the window
	UpdateWindow(hwnd);					// update the window

	done = false;						// intialize the loop condition variable


	Initialize();						// initialize OpenGL

	LARGE_INTEGER nFreq;
	LARGE_INTEGER nBeginTime;
	LARGE_INTEGER nEndTime;
	double time1, time2,time3;
	QueryPerformanceFrequency(&nFreq);

	//bitmapData2=new unsigned char[ width*height*3];
	//-------------文字设置----------------------------
	listBase = CreateBitmapFont("Arial", 48);

	//---------------------------------------


	// main message loop
	while (!done)
	{
		PeekMessage(&msg, hwnd, NULL, NULL, PM_REMOVE);

		if (msg.message == WM_QUIT)		// do we receive a WM_QUIT message?
		{
			done = true;				// if so, time to quit the application
		}
		else
		{
			if (keyPressed[VK_ESCAPE])
				done = true;
			else
			{
				//Render();
				// clear screen and depth buffer
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

				// -----------------------------------------------------------------
				/// 加载源图像 || 不在模糊的基础上模糊
				src = imread( "D:\\lufy512.bmp", 1 );
				//------------------------------------------------------------------
				QueryPerformanceCounter(&nBeginTime); 

				// 高斯模糊
				IMG_GaussBlur(src.data,bitmapData2,width,height,step,3);
				QueryPerformanceCounter(&nEndTime);
				time3=(double)(nEndTime.QuadPart-nBeginTime.QuadPart)*1000/(double)nFreq.QuadPart;
				// 颜色翻转
				for (int imageIdx = 0; imageIdx < width*height*3; imageIdx+=3)
				{
					unsigned char tempRGB;
					tempRGB = bitmapData2[imageIdx];
					bitmapData2[imageIdx] = bitmapData2[imageIdx + 2];
					bitmapData2[imageIdx + 2] = tempRGB;
				}

				//// 显示设置
				//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
				//glRasterPos2i(0,height-30);   // 位图相对于屏幕左下角的位置
				//glPixelZoom(1,-1);

				// 绘制图像
				glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, bitmapData2);

				SwapBuffers(g_HDC);			// bring backbuffer to foreground
				//DrawBitmap(width,height,src.data);
				//PrintString(listBase, "OpenGL Bitmap Fonts!");

				//SwapBuffers(g_HDC);			// bring backbuffer to foreground

				//SwapBuffers(g_HDC);			// bring backbuffer to foreground
				/// Create Windows
				namedWindow("Source Pic", 1);

				imshow( "Source Pic", src );
				waitKey(1);

				TranslateMessage(&msg);		// translate and dispatch to event queue
				DispatchMessage(&msg);
			}
		}
	}

	free(bitmapData);

	if (fullScreen)
	{
		ChangeDisplaySettings(NULL,0);		// If So Switch Back To The Desktop
		ShowCursor(TRUE);					// Show Mouse Pointer
	}

	return msg.wParam;
}



/****************************************
*    src        : 原始图像数据                *
*    dst        : 模糊后图像数据            *
*    width    : 图像宽                    *
*    height    : 图像高                    *
*    sigma    : 高斯参数                    *
*    chan    : 图像通道数                *
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


