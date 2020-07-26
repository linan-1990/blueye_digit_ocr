
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "Resource.h"

class Cocr_dllApp : public CWinApp
{
public:
	Cocr_dllApp();

public:
	virtual BOOL InitInstance();

	HWND m_hwndDlg;

	DECLARE_MESSAGE_MAP()
private:
	ULONG_PTR m_gdiplusToken;
public:
	virtual int ExitInstance();
};

extern Cocr_dllApp theApp;