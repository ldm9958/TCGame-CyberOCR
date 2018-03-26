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
#define LOG


string strLog = "";
string strLoginfo = "";
int g_maxAngle = 10;
//int g_iThreadCount = 0;

void Log(string str)
{	
#ifndef LOG
	return;
#endif // !LOG
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	string strCmd = stringformatA("echo %02d:%02d:%02d.%03d %s  >> %s",tm.wHour,tm.wMinute,tm.wSecond,tm.wMilliseconds, str.c_str(),strLog.c_str());
	if (string::npos == strCmd.find("&"))
	{
		system(strCmd.c_str());
	}
	cout << str << endl;	
}

std::string  stringformatA(const char* fmt, ...)
{
	std::string s = "";
	try
	{
		va_list   argptr;
		va_start(argptr, fmt);
#pragma warning( push )
#pragma warning( disable : 4996 )
		int   bufsize = _vsnprintf(NULL, 0, fmt, argptr) + 1;
#pragma warning( pop )
		char* buf = new char[bufsize];
		_vsnprintf_s(buf, bufsize, _TRUNCATE, fmt, argptr);
		s = buf;
		delete[] buf;
		va_end(argptr);
	}
	catch (...)
	{
		s = "TryError!";
	}
	return   s;
}

int stringreplace(std::string& strString, const char* src, const char* target)
{
	size_t nLen = 0;
	int iPos = 0;
	int iCount = 0;
	while (nLen < strString.length())
	{
		iPos = (int)strString.find(src, nLen);
		if (iPos >= 0)
		{//找到了
			iCount++;
			strString.replace(iPos, strlen(src), target);
			nLen = iPos + strlen(target);
		}
		else
		{
			break;
		}
	}
	return iCount;
}

wstring&   replace_all_distinct(wstring&   str, const   wstring&   old_value, const   wstring&   new_value)
{
	for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
		if ((pos = str.find(old_value, pos)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}

std::wstring  stringformat(const wchar_t* fmt, ...)
{
	std::wstring s = L"";
	try
	{
		va_list   argptr;
		va_start(argptr, fmt);
#pragma warning( push )
#pragma warning( disable : 4996 )
		int   bufsize = _vsnwprintf(NULL, 0, fmt, argptr) + 2;
#pragma warning( pop )
		wchar_t* buf = new wchar_t[bufsize];
		_vsnwprintf_s(buf, bufsize, _TRUNCATE, fmt, argptr);
		s = buf;
		delete[] buf;
		va_end(argptr);
	}
	catch (...)
	{
		s = L"TryError!";
	}
	return   s;
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

std::wstring Path_GetFileName(const wchar_t* strFileName)
{
	if (NULL == strFileName)
	{
		return L"";
	}

	std::wstring strPath = strFileName;
	int iLast = (int)strPath.rfind(L'\\');
	if (iLast > 0)
	{
		strPath.erase(0, iLast + 1);
	}
	if (strPath[0] == L'"')
	{
		strPath.erase(0, 1);
	}
	iLast = (int)strPath.find(L'\"');
	if (iLast > 0)
	{
		strPath.erase(iLast);
	}
	return strPath;
}

std::wstring Path_GetFileNameNoExt(const wchar_t* pFilePathName)
{
	wstring wstrFileName = Path_GetFileName(pFilePathName);
	if (wstrFileName.length() == 0)
	{
		return wstrFileName;
	}

	wchar_t szFileName[MAX_PATH] = { 0 };
	wcscpy_s(szFileName, MAX_PATH, wstrFileName.c_str());
	::PathRemoveExtension(szFileName);
	return szFileName;
}

std::wstring Path_GetFilePath(const wchar_t* strFileName)
{
	if (NULL == strFileName)
	{
		return L"";
	}
	std::wstring strPath = strFileName;
	int iLast = (int)strPath.rfind(L'\\');
	if (iLast > 0)
	{
		strPath.erase(iLast + 1);
	}
	return strPath;
}

void RemoveSpace(string &s)
{

	if (!s.empty())
	{
		s.erase(0, s.find_first_not_of(" "));
		s.erase(s.find_last_not_of(" ") + 1);
	}

}

int stringreplaceW(std::wstring& strString, const wchar_t* src, const wchar_t* target)
{
	size_t nLen = 0;
	int iPos = 0;
	int iCount = 0;
	while (nLen < strString.length())
	{
		iPos = (int)strString.find(src, nLen);
		if (iPos >= 0)
		{//找到了
			iCount++;
			strString.replace(iPos, wcslen(src), target);
			nLen = iPos + wcslen(target);
		}
		else
		{
			break;
		}
	}
	return iCount;
}

vector<wstring> g_vecInvalidWord = { L"\\",L"\/",L"：",L"*",L"?",L":",L"\"",L"<",L">",L"|" };
int IsFileNameValid(wstring& strFileName)
{
	if (strFileName.find(L"##"))
	{
		return -1;
	}

	if (stringreplaceW(strFileName, L"\\", L"#01") > 0)
	{
		Log("find \\");
	}
	if (stringreplaceW(strFileName, L"/", L"#02") > 0)
	{
		Log("find /");
	}
	if (stringreplaceW(strFileName, L":", L"#03") > 0)
	{
		Log("find :");
	}
	if (stringreplaceW(strFileName, L"*", L"#04") > 0)
	{
		Log("find *");
	}
	if (stringreplaceW(strFileName, L"?", L"#05") > 0)
	{
		Log("find ?");
	}
	if (stringreplaceW(strFileName, L"：", L"#03") > 0)
	{
		Log("find :");
	}
	if (stringreplaceW(strFileName, L"\"", L"#06") > 0)
	{
		Log("find \"");
	}
	if (stringreplaceW(strFileName, L"<", L"#07") > 0)
	{
		Log("find <");
	}
	if (stringreplaceW(strFileName, L">", L"#08") > 0)
	{
		Log("find >");
	}
	if (stringreplaceW(strFileName, L"|", L"#09") > 0)
	{
		Log("find |");
	}

	return 0;
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

int ScreenShotImgByPlots(std::string strImgPath, TXT_LINE_INFO info,wstring strSavePath,DWORD & dwCount)
{
	strLoginfo = stringformatA("Enter ScreenShotImgByPlots image:%s character=%s",strImgPath.c_str(),wstr2str(info.strInfo,CP_ACP).c_str());
	Log(strLoginfo);
	wstring strTemp = str2wstr(strImgPath, CP_UTF8);
	strImgPath = wstr2str(replace_all_distinct(Path_GetFilePath(strTemp.c_str()), L"txt", L"image") + Path_GetFileNameNoExt(strTemp.c_str()) + L".jpg", CP_ACP);
	if (!::PathFileExists(str2wstr(strImgPath, CP_ACP).c_str()))
	{
		strLoginfo = stringformatA("File %s doesn't exist!!",strImgPath.c_str());
		Log(strLoginfo);
		return ERROR_FILE_NOT_EXIST;
	}
	if (info.vecPlots.size() != 8)
	{
		Log("Error vector  size ..");
		return ERROR_INVALID_PARAM;
	}
	
	//有几个数据竟然是负的，坑爹！
	double iPlotA_x = info.vecPlots[0]>0 ? info.vecPlots[0] : 0;
	double iPlotA_y = info.vecPlots[1]>0 ? info.vecPlots[1] : 0;
	double iPlotB_x = info.vecPlots[2]>0 ? info.vecPlots[2] : 0;
	double iPlotB_y = info.vecPlots[3]>0 ? info.vecPlots[3] : 0;
	double iPlotC_x = info.vecPlots[4]>0 ? info.vecPlots[4] : 0;
	double iPlotC_y = info.vecPlots[5]>0 ? info.vecPlots[5] : 0;
	double iPlotD_x = info.vecPlots[6]>0 ? info.vecPlots[6] : 0;
	double iPlotD_y = info.vecPlots[7]>0 ? info.vecPlots[7] : 0;

	int iXmin = MIN(MIN(MIN(iPlotA_x, iPlotB_x), iPlotC_x), iPlotD_x);
	int iXmax = MAX(MAX(MAX(ceil(iPlotA_x), ceil(iPlotB_x)), ceil(iPlotC_x)), ceil(iPlotD_x));
	int iYmin = MIN(MIN(MIN(iPlotA_y, iPlotB_y), iPlotC_y), iPlotD_y);
	int iYmax = MAX(MAX(MAX(ceil(iPlotA_y), ceil(iPlotB_y)), ceil(iPlotC_y)), ceil(iPlotD_y));

	strLoginfo = stringformatA("iXmin = %d,iXmax = %d, iYmin = %d, iYmax = %d",iXmin,iXmax,iYmin,iYmax);
	Log(strLoginfo);
	int iLength, iHeight = 0;
	double DeltaX, DeltaY = 0.0;
	Mat imgShot;
	BOOL bNeedTrans = FALSE;

	strLoginfo = stringformatA("Before computing ...");
	Log(strLoginfo);
	//计算生成的图片长与宽
	iLength = (int)sqrt((iPlotA_x - iPlotD_x)*(iPlotA_x - iPlotD_x) + (iPlotA_y - iPlotD_y)*(iPlotA_y - iPlotD_y));
	iHeight = (int)sqrt((iPlotA_x - iPlotB_x)*(iPlotA_x - iPlotB_x) + (iPlotA_y - iPlotB_y)*(iPlotA_y - iPlotB_y));

	Rect roi = Rect(iXmin, iYmin, iXmax - iXmin, iYmax - iYmin);
	strLoginfo = stringformatA("ROI height = %d, width = %d, iLength = %d ,iHeight = %d",roi.height,roi.width,iLength,iHeight);
	Log(strLoginfo);

	if (iLength == 0 || iHeight == 0)
	{
		strLoginfo = stringformatA("File %s param error",strImgPath.c_str());
		Log(strLoginfo);
		return ERROR_INVALID_PARAM;
	}
	strLoginfo = stringformatA("After computing ...  ");
	Log(strLoginfo);
	//对应X轴Y轴偏移量。
	DeltaX = (iPlotD_x - iPlotA_x) / iLength;
	DeltaY = (iPlotD_y - iPlotA_y) / iHeight;

	strLoginfo = stringformatA("DeltaX = %.4f, DeltaY = %.4f",DeltaX,DeltaY);
	Log(strLoginfo);
	do
	{		
		Mat img = imread(strImgPath.c_str());
		if (iXmax > img.cols || iYmax > img.rows)
		{
			Log("Invalid shot size");
			return -1;
		}		
		if (!img.data)
		{
			strLoginfo = stringformatA("Read file %s error",strImgPath.c_str());
			Log(strLoginfo);
			return -1;
		}
		
		double angle = atan((iPlotD_y - iPlotA_y) / (iPlotD_x - iPlotA_x)) * 180 / PI;
		if (angle > g_maxAngle || angle < -g_maxAngle)
		{
			Log("invalid angle");
			return ERROR_INVALID_PARAM;
		}
		Mat roiImg = img(roi);
		Point center = Point(roiImg.cols / 2, roiImg.rows / 2);
		
		double scale = 1;
		strLoginfo = stringformatA("Before getRotationMatrix2D ...");
		Log(strLoginfo);
		Mat rot_mat = getRotationMatrix2D(center, angle, scale);
		strLoginfo = stringformatA("Before warpAffine ...");
		Log(strLoginfo);
		if (roiImg.cols <= 0 || roiImg.rows <= 0)
		{
			Log("Error roiImg size..");
			return -1;
		}
		warpAffine(roiImg, roiImg, rot_mat, roiImg.size());
		strLoginfo = stringformatA("After warpAffine ...");
		Log(strLoginfo);
		try
		{
			IsFileNameValid(info.strInfo);
			string strRealSavePath = stringformatA("%s\\%d_%s.jpg", UTF8ToGB(wstr2str(strSavePath, CP_UTF8).c_str()), dwCount, UTF8ToGB(wstr2str(info.strInfo, CP_UTF8).c_str()));
			imwrite(strRealSavePath.c_str(), roiImg);
			strLoginfo = stringformatA("Writer image successfully %s",strRealSavePath.c_str());
			Log(strLoginfo);
			dwCount++;
		}
		catch (const std::exception&)
		{
			Log("write file error");
		}		

	} while (FALSE);

	return 0;
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
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	strLog = stringformatA("c:\\log__%02d_%02d_%02d_%02d_%02d_%02d.txt", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
	Log("start");

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
	if (IDOK != ::MessageBox(0, strMsg.c_str(),L"提示", MB_OKCANCEL))
	{
		return -1;
	}

	string strTarget = ".txt";
	strLoginfo = stringformatA("Scan all %s files in %s", strTarget.c_str(), wstr2str(strPath, CP_ACP).c_str());
	Log(strLoginfo);

	//获取路径下全部Txt文件
	GetFoldAllDecFile(strPath, vecAllFile, str2wstr(strTarget, CP_UTF8));
	strLoginfo = stringformatA("Scan finished  , find %d files totally", vecAllFile.size());

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
		strLoginfo = stringformatA("No.%d file ,Start dealing with file %s",k, wstr2str(*iter, CP_ACP).c_str());
		Log(strLoginfo);
		while (std::getline(ifile, line)) {
			strLoginfo = stringformatA("    %d line :%s",i, UTF8ToGB(line.c_str()).c_str());
			Log(strLoginfo);
			if (0 != SplitTxtInfo(str2wstr(line).c_str(), info))
			{
				Log("split error, continue!");
				continue;
			}
			if (0 != ScreenShotImgByPlots(wstr2str(*iter, CP_ACP).c_str(), info, strSavePath, dwCount))
			{
				Log("ScreenShotImgByPlots error, continue!");
				continue;
			}			
			i++;
		}
		strLoginfo = stringformatA("End dealing with file %s", wstr2str(*iter, CP_ACP).c_str());
		Log(strLoginfo);
		ifile.close();
	}

	system("pause ");
	return 0;
}

