#include "stdafx.h"
#include "ocr_dllApi.h"

_declspec(dllexport) void showDlg(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//ocr_dllDialog dlldlg;
	//dlldlg.DoModal();
	CCatchScreenDlg dlg;
	dlg.DoModal();
}