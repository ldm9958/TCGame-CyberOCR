// MyCyberOCR.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <shlwapi.h>
#include <windows.h>
#include <iostream>
#include <math.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv/highgui.h"
#include "Common.h"

using namespace std;
using namespace cv;

#define PI		3.14159265

std::string wstr2str(const std::wstring wstrSrc, UINT CodePage/*=CP_ACP CP_UTF8*/)
{
	if (wstrSrc.length() == 0)
		return "";

	//得到转化后需要Buf的长度
	std::string retn = "";
	try
	{
		int buflen = ::WideCharToMultiByte(CodePage, 0, wstrSrc.c_str(), -1, NULL, 0, NULL, NULL) + 1;
		if (buflen == 0)
			return "";
		char * buf = new char[buflen];
		if (buf != NULL)
		{
			memset(buf, 0, buflen);
			::WideCharToMultiByte(CodePage, 0, wstrSrc.c_str(), -1, buf, buflen, NULL, NULL);
			retn = buf;
			delete[]buf;
		}
	}
	catch (...)
	{

	}
	return retn;

}

void GetFoldAllDecFile(wstring strFold, vector<wstring> &vecFile, wstring strTarget)
{
	wstring strFind = strFold + L"\\*.txt";
	WIN32_FIND_DATA FileData;
	DWORD dwAttrs = 0;
	HANDLE hFind = ::FindFirstFile(strFind.c_str(), &FileData);


	while (INVALID_HANDLE_VALUE != hFind)
	{
		wstring str(FileData.cFileName);
		wstring strTemp = strFold + L"\\" + FileData.cFileName;

		dwAttrs = ::GetFileAttributes(strTemp.c_str());
		if (INVALID_FILE_ATTRIBUTES == dwAttrs)
		{
		}
		else if (dwAttrs & FILE_ATTRIBUTE_DIRECTORY)
		{
			if ((0 != wcscmp(FileData.cFileName, L".")) && (0 != wcscmp(FileData.cFileName, L"..")))
			{
				GetFoldAllDecFile(strTemp, vecFile, strTarget);
			}
		}
		else
		{
			wcout << L"add file " << strTemp.c_str() << endl;
			vecFile.push_back(strTemp);
		}

		if (!::FindNextFile(hFind, &FileData))
		{
			break;
		}
	}
	if (INVALID_HANDLE_VALUE != hFind)
	{
		::FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
	}
}


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
	if (!::PathFileExists(str2wstr(strImgPath, CP_UTF8).c_str()))
	{
		std::cout << "File" << strImgPath.c_str() << "doesn't exist!!" << std::endl;
		return ERROR_FILE_NOT_EXIST;
	}
	int iXmin = MIN(MIN(MIN(iPlotA_x, iPlotB_x), iPlotC_x), iPlotD_x);
	int iXmax = MAX(MAX(MAX(ceil(iPlotA_x), ceil(iPlotB_x)), ceil(iPlotC_x)), ceil(iPlotD_x));
	int iYmin = MIN(MIN(MIN(iPlotA_y, iPlotB_y), iPlotC_y), iPlotD_y);
	int iYmax = MAX(MAX(MAX(ceil(iPlotA_y), ceil(iPlotB_y)), ceil(iPlotC_y)), ceil(iPlotD_y));

	cout << "iXmin = " << iXmin << "iXmax = " << iXmax << "iYmin = " << iYmin << "iYMax = " << iYmax << endl;
	int iLength, iHeight = 0;
	double DeltaX, DeltaY = 0.0;
	Mat imgShot;
	BOOL bNeedTrans = FALSE;	
	
	//计算生成的图片长与宽
	iLength = (int)sqrt((iPlotA_x - iPlotD_x)*(iPlotA_x - iPlotD_x) + (iPlotA_y - iPlotD_y)*(iPlotA_y - iPlotD_y));
	iHeight = (int)sqrt((iPlotA_x - iPlotB_x)*(iPlotA_x - iPlotB_x) + (iPlotA_y - iPlotB_y)*(iPlotA_y - iPlotB_y));

	Rect roi = Rect(iXmin, iYmin, iXmax - iXmin, iYmax - iYmin);
	cout << "height" << roi.height << "width" << roi.width << endl;
	cout << "iLength=" << iLength << "  iHeight=" << iHeight << endl;

	if (iLength == 0 || iHeight == 0)
	{
		std::cout << "File" << strImgPath.c_str() << "param error!!" << std::endl;
		return ERROR_INVALID_PARAM;
	}

	//对应X轴Y轴偏移量。
	DeltaX = (iPlotD_x - iPlotA_x) / iLength;
	DeltaY = (iPlotD_y - iPlotA_y) / iHeight;
	
	cout << "DeltaX=" << DeltaX << "  DeltaY=" << DeltaY << endl;
	do
	{	
		Mat img = imread(strImgPath.c_str());
		Mat roiImg = img(roi);
			
		Point center = Point(roiImg.cols / 2, roiImg.rows / 2);
		double angle = atan((iPlotD_y-iPlotA_y)/(iPlotD_x-iPlotA_x))*180/PI;
		double scale = 1;
		Mat rot_mat = getRotationMatrix2D(center, angle, scale);
		warpAffine(roiImg, roiImg, rot_mat, roiImg.size());

		imwrite("D://my.jpg", roiImg);

	} while (FALSE);
	::MessageBox(0, 0,L"hello", 0);
	
}

int _tmain(int argc, _TCHAR* argv[])
{
	wstring strPath = L"";
	vector<wstring> vecAllFile;
	vecAllFile.clear();
	if (argv[1])
	{
		strPath = argv[1];
	}
	else
	{
		strPath = L"D:\\ml\\train_1000\\txt_1000";
	}
	string strTarget = ".txt";
	cout << "Scan all " << strTarget.c_str() << "files in " << wstr2str(strPath, CP_ACP).c_str() << endl;
	GetFoldAllDecFile(strPath, vecAllFile, str2wstr(strTarget, CP_UTF8));

	cout << "Scan finished , find " << vecAllFile.size() << "files totally.." << endl;
	if (vecAllFile.size() == 0)
	{
		return -1;
	}
	vector<wstring>::iterator iter = vecAllFile.begin();
	int k = 1;
	for (; iter != vecAllFile.end(); iter++, k++)
	{

	}
	ScreenShotImgByPlots("D:\\ml\\train_1000\\image_1000\\TB1..FLLXXXXXbCXpXXunYpLFXX.jpg", 407.6, 413.6, 407.6, 425.6, 598.0, 411.2, 600.4, 397.6);
	system("pause ");
	return 0;
}

