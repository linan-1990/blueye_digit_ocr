
#include "stdafx.h"
#include "ocr_dll.h"
#include "ocr_dllDialog.h"
#include "CatchScreenDlg.h"
#include <GdiPlus.h>

using namespace Gdiplus;
#pragma comment(lib,"GdiPlus.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(Cocr_dllApp, CWinApp)
END_MESSAGE_MAP()



Cocr_dllApp::Cocr_dllApp()
{
	
}

Cocr_dllApp theApp;


BOOL Cocr_dllApp::InitInstance()
{
	CWinApp::InitInstance();

	GdiplusStartupInput input;
	GdiplusStartup(&m_gdiplusToken, &input, NULL);

	return TRUE;
}

int Cocr_dllApp::ExitInstance()
{
	GdiplusShutdown(m_gdiplusToken);
	return CWinApp::ExitInstance();
}