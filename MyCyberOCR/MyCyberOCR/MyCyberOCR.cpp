// MyCyberOCR.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <shlwapi.h>
#include <windows.h>
#include <iostream>
#include <math.h>
#include "opencv2/highgui.hpp"
#include "opencv/highgui.h"
#include "Common.h"

using namespace std;
using namespace cv;

std::wstring str2wstr(const std::string wstrSrc, UINT CodePage = CP_UTF8)
{
	if (wstrSrc.length() == 0)
		return L"";

	int buflen = MultiByteToWideChar(CodePage, 0, wstrSrc.c_str(), -1, NULL, 0) + 1;
	if (buflen == 0)
		return L"";

	wchar_t * buf = new wchar_t[buflen];
	std::wstring retn = L"";
	if (buf)
	{
		memset(buf, 0, buflen * 2);
		::MultiByteToWideChar(CodePage, 0, wstrSrc.c_str(), -1, buf, buflen);
		retn = buf;
		delete[]buf;
	}
	return retn;
}

int ScreenShotImgByPlots(std::string strImgPath, double iPlotA_x, double iPlotA_y, double iPlotB_x, double iPlotB_y, double iPlotC_x, double iPlotC_y, double iPlotD_x, double iPlotD_y)
{
	if (!::PathFileExists(str2wstr(strImgPath,CP_UTF8).c_str()))
	{
		std::cout << "File" << strImgPath.c_str() << "doesn't exist!!" <<std::endl;
		return ERROR_FILE_NOT_EXIST;
	}

	int iLength,iHeight		= 0;
	double DeltaX,DeltaY	= 0.0;

	iLength = (int)sqrt((iPlotA_x - iPlotD_x)*(iPlotA_x - iPlotD_x) + (iPlotA_y - iPlotD_y)*(iPlotA_y - iPlotD_y));		//计算生成的图片长与宽
	iHeight = (int)sqrt((iPlotA_x - iPlotB_x)*(iPlotA_x - iPlotB_x) + (iPlotA_y - iPlotB_y)*(iPlotA_y - iPlotB_y));
	cout << "iLength=" << iLength << "  iHeight=" << iHeight << endl;

	if (iLength == 0 || iHeight == 0)
	{
		std::cout << "File" << strImgPath.c_str() << "param error!!" << std::endl;
		return ERROR_INVALID_PARAM;
	}
	DeltaX = (iPlotD_x - iPlotA_x) / iLength;																			//得出倾斜角Cos x与Sin x 对应X轴Y轴偏移量。
	DeltaY = (iPlotD_y - iPlotA_y) / iHeight;

	cout << "DeltaX=" << DeltaX << "  DeltaY=" << DeltaY << endl;
	do
	{
		IplImage * img = cvLoadImage(strImgPath.c_str());
		std::cout << "width" << img->width << "height" << img->height << std::endl;
		Mat imgShot = Mat(iHeight, iLength, CV_8UC3);
		int nr = imgShot.rows;
		 // 将3通道转换为1通道
		int nl = imgShot.cols;

		for (int k = 0; k < nr; k++)
		{
			// 每一行图像的指针

			for (int i = 0; i < nl; i++)
			{
			//	CV_MAT_ELEM(imgShot, Mat, k, i) = (img->imageData + (int)floor(k + DeltaY*k)*img->widthStep)[(int)floor(i + DeltaX*i)];
			//	imgShot.at<Mat>(i,k) = (img->imageData + (int)floor(k + DeltaY*k)*img->widthStep)[(int)floor(i + DeltaX*i)];
			}
		}

		imwrite("my.jpg", imgShot);
		
	} while (FALSE);
	
}

int _tmain(int argc, _TCHAR* argv[])
{
	ScreenShotImgByPlots("D:\\ml\\train_1000\\image_1000\\TB1..FLLXXXXXbCXpXXunYpLFXX.jpg", 407.6,413.6,407.6,425.6,598.0,411.2,600.4,397.6);
	system("pause ");
	return 0;
}

