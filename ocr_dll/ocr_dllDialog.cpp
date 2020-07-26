
#include "stdafx.h"
#include "ocr_dll.h"
#include "ocr_dllDialog.h"
#include "afxdialogex.h"
#include "CatchScreenDlg.h"


IMPLEMENT_DYNAMIC(ocr_dllDialog, CDialog)

ocr_dllDialog::ocr_dllDialog(CWnd* pParent /*=NULL*/)
	: CDialog(ocr_dllDialog::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_BLUEYE);
}

ocr_dllDialog::~ocr_dllDialog()
{
}

void ocr_dllDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ocr_dllDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &ocr_dllDialog::OnBnClickedButton1)
END_MESSAGE_MAP()

BOOL ocr_dllDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	return TRUE;
}

UINT SccreenShot_Thread(LPVOID lpParam)
{
	HWND hWndMain = (HWND)lpParam;
	CCatchScreenDlg dlg;
	dlg.DoModal();

	::ShowWindow(hWndMain, SW_SHOW);
	return 0;
}

void ocr_dllDialog::OnBnClickedButton1()
{
	::ShowWindow(m_hWnd, SW_HIDE);
	::AfxBeginThread(SccreenShot_Thread, (LPVOID)GetSafeHwnd());
}
