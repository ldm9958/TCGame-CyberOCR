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
#include <fstream>

using namespace std;
using namespace cv;

#define PI		3.14159265
#define  SAVE_PATH	 L"D:\\ml\\train"

wstring&   replace_all_distinct(wstring&   str, const   wstring&   old_value, const   wstring&   new_value)
{
	for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
		if ((pos = str.find(old_value, pos)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}

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
wstring PathGetFileNameNoExt(wstring strFilePath)
{
	return strFilePath;
}

wstring PathGetFilePath(wstring strFilePath)
{
	return strFilePath;
}


int ScreenShotImgByPlots(std::string strImgPath, TXT_LINE_INFO info,wstring strSavePath,DWORD dwCount)
{
	wstring strTemp = str2wstr(strImgPath, CP_UTF8);
	strImgPath = wstr2str(replace_all_distinct(PathGetFilePath(strTemp), L"txt", L"image") + PathGetFileNameNoExt(strTemp) + L".jpg", CP_ACP);
	if (!::PathFileExists(str2wstr(strImgPath, CP_ACP).c_str()))
	{
		std::cout << "File" << strImgPath.c_str() << "doesn't exist!!" << std::endl;
		return ERROR_FILE_NOT_EXIST;
	}
	if (info.vecPlots.size() != 8)
	{
		std::cout << "Error vector  size .." << endl;
		return ERROR_INVALID_PARAM;
	}

	double iPlotA_x = info.vecPlots[0];
	double iPlotA_y = info.vecPlots[1];
	double iPlotB_x = info.vecPlots[2];
	double iPlotB_y = info.vecPlots[3];
	double iPlotC_x = info.vecPlots[4];
	double iPlotC_y = info.vecPlots[5];
	double iPlotD_x = info.vecPlots[6];
	double iPlotD_y = info.vecPlots[7];

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
		double angle = atan((iPlotD_y - iPlotA_y) / (iPlotD_x - iPlotA_x)) * 180 / PI;
		double scale = 1;
		Mat rot_mat = getRotationMatrix2D(center, angle, scale);
		warpAffine(roiImg, roiImg, rot_mat, roiImg.size());

		try
		{
			char *s;
			sprintf(s,"%s\\%d_%s.jpg", wstr2str(strSavePath,CP_ACP).c_str(), dwCount,info.strInfo);
			cout << s << endl;
			imwrite(s, roiImg);
		}
		catch (const std::exception&)
		{
			cout << "imwrite error" << endl;
		}		

	} while (FALSE);
	::MessageBox(0, 0, L"hello", 0);

}

string UTF8ToGB(const char* str)
{
	string result;
	WCHAR *strSrc;
	LPSTR szRes;

	//获得临时变量的大小
	int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	strSrc = new WCHAR[i + 1];
	MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, i);

	//获得临时变量的大小
	i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
	szRes = new CHAR[i + 1];
	WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);

	result = szRes;
	delete[]strSrc;
	delete[]szRes;

	return result;
}

int SplitTxtInfo(wstring strTxtLine, TXT_LINE_INFO &info)
{
	size_t index = string::npos;
	size_t iTmp = string::npos;
	info.vecPlots.clear();
	for (int i= 0;i<8;i++)
	{
		index = strTxtLine.find(L",", index + 1);
		if (index == string::npos)
		{
			return -1;
		}
		info.vecPlots.push_back(_wtof(strTxtLine.substr(iTmp + 1, index - iTmp - 1).c_str()));		
		wcout << strTxtLine.substr(iTmp + 1, index - iTmp - 1).c_str() << endl;
		iTmp = index;
	}
	info.strInfo = strTxtLine.erase(0, index +1);
	wcout << info.strInfo << endl;
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD dwCount = 0;
	wstring strPath = L"";
	wstring strSavePath = L"D:\\ml\\train";
	vector<wstring> vecAllFile;
	vecAllFile.clear();
	if (argv[1])
	{
		strPath = argv[1];
		if (argv[2])
		{
			strSavePath = argv[2];
		}
	}
	else
	{
		strPath = L"D:\\ml\\train_1000\\txt_1000";
		//strPath = L"F:\\spt\\学习资料\\ml\\train_1000\\txt_1000";
	}

	wstring strMsg = L"TXT文件存储路径为：" + strPath + L"图片存储路径为：" + strSavePath + L"\n 是否立即开始处理训练集?";	
	if (IDOK != ::MessageBox(0, strMsg.c_str(),L"提示",0))
	{
		return -1;
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
	ifstream ifile;
	string line;
	int i;
	TXT_LINE_INFO info;
	for (; iter != vecAllFile.end(); iter++, k++)
	{
		i = 1;
		ifile.open(wstr2str(*iter, CP_ACP).c_str());
		cout << "Open " << k << " file " << wstr2str(*iter, CP_ACP).c_str() << endl;

		while (std::getline(ifile, line)) {
			cout <<"	"<< i << "line" << UTF8ToGB(line.c_str()).c_str() << endl;
			SplitTxtInfo(str2wstr(UTF8ToGB(line.c_str()).c_str()), info);
			ScreenShotImgByPlots(wstr2str(*iter, CP_ACP).c_str(), info , strSavePath ,dwCount);
			i++;
		}
		ifile.close();
	}

	system("pause ");
	return 0;
}

