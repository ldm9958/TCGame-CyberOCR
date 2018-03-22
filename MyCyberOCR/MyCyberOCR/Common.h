#pragma once

#include "stdafx.h"
#include <string>
#include <vector>

using namespace std;

#define ERROR_INVALID_PARAM		-1
#define ERROR_FILE_NOT_EXIST	-2

typedef struct txtInfo
{
	vector<double> vecPlots;	
	DWORD dwLine;
	wstring strFileName;
	wstring strInfo;

	txtInfo()
	{
		vecPlots.clear();
		dwLine = 0;
		strFileName	= L"";
		strInfo		= L"";
	}
}TXT_LINE_INFO,* PTXT_LINE_INFO;
