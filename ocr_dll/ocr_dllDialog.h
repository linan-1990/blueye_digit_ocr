#pragma once

#include "Resource.h"

class ocr_dllDialog : public CDialog
{
	DECLARE_DYNAMIC(ocr_dllDialog)

public:
	ocr_dllDialog(CWnd* pParent = NULL);
	virtual ~ocr_dllDialog();

	enum { IDD = IDD_MFCTESTDIALOG };

protected:
	HICON m_hIcon;
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
};
