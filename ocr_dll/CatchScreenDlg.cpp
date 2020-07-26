#include "stdafx.h"
#include "ocr_dll.h"
#include "CatchScreenDlg.h"

#include <GdiPlus.h>
using namespace  Gdiplus;

#include "digitOCR.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CCatchScreenDlg dialog

CCatchScreenDlg::CCatchScreenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCatchScreenDlg::IDD, pParent)
{
	m_bLBtnDown = FALSE;
	//初始化像皮筋类,新增的resizeMiddle 类型
	m_rectTracker.m_nStyle = CMyTracker::resizeMiddle | CMyTracker::solidLine;
	m_rectTracker.m_rect.SetRect(-1, -2, -3, -4);
	//设置矩形颜色
	m_rectTracker.SetRectColor(RGB(10, 100, 130));

	m_hCursor = AfxGetApp()->LoadCursor(IDC_CURSOR1);

	m_bDraw = FALSE;
	m_bFirstDraw = TRUE;
	m_bQuit = FALSE;
	m_bNeedShowMsg = FALSE;
	m_startPt = 0;

	//获取屏幕分辩率
	m_xScreen = GetSystemMetrics(SM_CXSCREEN);
	m_yScreen = GetSystemMetrics(SM_CYSCREEN);

	//截取屏幕到位图中
	CRect rect(0, 0, m_xScreen, m_yScreen);
	m_hBitmap = CopyScreenToBitmap(&rect);
	m_pBitmap = CBitmap::FromHandle(m_hBitmap);

	//初始化刷新窗口区域 m_rgn
	m_rgn.CreateRectRgn(0, 0, 0, 0);
}

void CCatchScreenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCatchScreenDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//ON_WM_LBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	//ON_WM_RBUTTONUP()
	ON_WM_CTLCOLOR()
	//ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCatchScreenDlg message handlers

BOOL CCatchScreenDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//把对化框设置成全屏顶层窗口
	SetWindowPos(&wndTopMost, 0, 0, m_xScreen, m_yScreen, SWP_SHOWWINDOW);

	m_toolBar.CreateToolBar(m_hWnd);
	m_toolBar.RemoveChildStyle();
	::MoveWindow(m_toolBar.GetHWND(), 2000, 2000, 51, 29, FALSE);

	((Cocr_dllApp*)AfxGetApp())->m_hwndDlg = AfxGetMainWnd()->GetSafeHwnd();
	return TRUE;
}

void CCatchScreenDlg::OnPaint()
{
	// 如果窗口是最小化状态
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

	}
	else  // 如果窗口正常显示
	{
		CPaintDC dc(this);

		CDC dcCompatible;

		dcCompatible.CreateCompatibleDC(&dc);
		RECT rect;
		::GetClientRect(m_hWnd, &rect);
		HBITMAP hBitmap = ::CreateCompatibleBitmap(dc.m_hDC, rect.right - rect.left, rect.bottom - rect.top);
		::SelectObject(dcCompatible.m_hDC, hBitmap);

		HBRUSH s_hBitmapBrush = CreatePatternBrush(m_hBitmap);
		::FillRect(dcCompatible.m_hDC, &rect, s_hBitmapBrush);

		//画出像皮筋矩形
		if (m_bFirstDraw)
		{
			m_rectTracker.Draw(&dcCompatible);
		}
		Gdiplus::Graphics graphics(dcCompatible);

		HRGN hgn1 = CreateRectRgn(m_rectTracker.m_rect.left, m_rectTracker.m_rect.top,
			m_rectTracker.m_rect.right, m_rectTracker.m_rect.bottom);
		Region region1(hgn1);

		HRGN hgn2 = CreateRectRgn(rect.left, rect.top,
			rect.right, rect.bottom);
		Region region2(hgn2);

		region2.Exclude(&region1);

		SolidBrush  solidBrush(Color(100, 128, 128, 128));
		graphics.FillRegion(&solidBrush, &region2);

		DeleteObject(hgn1);
		DeleteObject(hgn2);

		dc.BitBlt(0, 0, rect.right, rect.bottom, &dcCompatible, 0, 0, SRCCOPY);
		DeleteObject(hBitmap);
		DeleteObject(s_hBitmapBrush);

		//CDialog::OnPaint();
	}
}

void CCatchScreenDlg::OnCancel()
{
	if (m_bFirstDraw)
	{
		//取消已画矩形变量
		m_bFirstDraw = FALSE;
		m_bDraw = FALSE;
		m_rectTracker.m_rect.SetRect(-1, -1, -1, -1);
		InvalidateRgnWindow();
	}
	else
	{
		CDialog::OnCancel();
	}
}

void CCatchScreenDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bLBtnDown)
		m_toolBar.HideToolBar();
	else
	{
		if (m_bFirstDraw) m_toolBar.HideToolBar();
		else m_toolBar.ShowToolBar();
	}

	if (m_bDraw)
	{
		//动态调整矩形大小,并刷新画出
		m_rectTracker.m_rect.SetRect(m_startPt.x + 4, m_startPt.y + 4, point.x, point.y);
		InvalidateRgnWindow();
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CCatchScreenDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bLBtnDown = TRUE;
	int nHitTest;
	nHitTest = m_rectTracker.HitTest(point);
	//判断击中位置
	if (nHitTest < 0)
	{
		if (m_bFirstDraw)
		{
			//第一次画矩形
			m_startPt = point;
			m_bDraw = TRUE;
			m_bFirstDraw = FALSE;
			//设置当鼠标按下时最小的矩形大小
			m_rectTracker.m_rect.SetRect(point.x, point.y, point.x + 4, point.y + 4);

			InvalidateRgnWindow();
		}
	}
	else
	{
		//保证当鼠标当下时立刻显示信息
		m_bNeedShowMsg = FALSE;
		if (m_bFirstDraw)
		{
			//调束大小时,Track会自动调整矩形大小,在些期间,消息归CRectTracker内部处理
			m_rectTracker.Track(this, point, TRUE);
			InvalidateRgnWindow();
		}
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CCatchScreenDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bLBtnDown = FALSE;
	m_bNeedShowMsg = FALSE;
	m_bDraw = FALSE;

	m_toolBar.SetShowPlace(m_rectTracker.m_rect.right, m_rectTracker.m_rect.bottom);

	InvalidateRgnWindow();
	CDialog::OnLButtonUp(nFlags, point);
}

/*
void CCatchScreenDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int nHitTest;
	nHitTest = m_rectTracker.HitTest(point);

	//如果在是矩形内部双击
	if (nHitTest == 8)
	{
		//保存位图到粘贴板中,bSave 为TRUE,
		CopyScreenToBitmap(m_rectTracker.m_rect, TRUE);
		PostQuitMessage(0);
	}

	CDialog::OnLButtonDblClk(nFlags, point);
}


void CCatchScreenDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_toolBar.HideToolBar();
	//InvalidateRgnWindow();
	CDialog::OnRButtonDown(nFlags, point);
}

void CCatchScreenDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_bLBtnDown = FALSE;
	if (m_bFirstDraw)
	{
		//如果已经截取矩则清除截取矩形
		m_bFirstDraw = FALSE;
		//清除矩形大小
		m_rectTracker.m_rect.SetRect(-1, -1, -1, -1);
		InvalidateRgnWindow();
	}
	else
	{
		//关闭程序
		PostQuitMessage(0);
	}

	CDialog::OnRButtonUp(nFlags, point);
}
*/

HBRUSH CCatchScreenDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	//设置操作提示窗口文本颜色
	if (pWnd->GetDlgCtrlID() == IDC_EDIT1)
	{
		pDC->SetTextColor(RGB(247, 76, 128));
	}

	return hbr;
}

BOOL CCatchScreenDlg::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;

	BITMAP bmp;
	m_pBitmap->GetBitmap(&bmp);

	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(pDC);

	dcCompatible.SelectObject(m_pBitmap);

	CRect rect;
	GetClientRect(&rect);
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dcCompatible, 0, 0, SRCCOPY);
	return TRUE;
}

BOOL CCatchScreenDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	//设置改变截取矩形大小时光标
	if (pWnd == this && m_rectTracker.SetCursor(this, nHitTest)
		&& !m_bDraw && m_bFirstDraw) //此处判断保截取时当标始中为彩色光标
	{
		return TRUE;
	}

	//设置彩色光标
	SetCursor(m_hCursor);
	return TRUE;
}

Mat bmp2mat(HDC hDC, HBITMAP hbwindow) {

	HDC hwindowDC, hwindowCompatibleDC;

	BITMAP bitmapInfo;
	GetObject(hbwindow, sizeof(BITMAP), &bitmapInfo);

	Mat src;
	BITMAPINFOHEADER bi;

	hwindowDC = hDC;
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	src.create(bitmapInfo.bmHeight, bitmapInfo.bmWidth, CV_8UC4);

	// create a bitmap
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bitmapInfo.bmWidth;
	bi.biHeight = -bitmapInfo.bmHeight;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, bitmapInfo.bmHeight, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	// avoid memory leak
	DeleteDC(hwindowCompatibleDC); DeleteDC(hwindowDC);

	return src;
}

// 拷贝屏幕, 这段源码源自CSDN
// lpRect 代表选定区域
HBITMAP CCatchScreenDlg::CopyScreenToBitmap(LPRECT lpRect, BOOL bSave)

{
	HDC       hScrDC, hMemDC;
	// 屏幕和内存设备描述表
	HBITMAP    hBitmap, hOldBitmap;
	// 位图句柄
	int       nX, nY, nX2, nY2;
	// 选定区域坐标
	int       nWidth, nHeight;

	// 确保选定区域不为空矩形
	if (IsRectEmpty(lpRect))
		return NULL;
	//为屏幕创建设备描述表
	hScrDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	//为屏幕设备描述表创建兼容的内存设备描述表
	hMemDC = CreateCompatibleDC(hScrDC);
	// 获得选定区域坐标
	nX = lpRect->left;
	nY = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;

	//确保选定区域是可见的
	if (nX < 0)
		nX = 0;
	if (nY < 0)
		nY = 0;
	if (nX2 > m_xScreen)
		nX2 = m_xScreen;
	if (nY2 > m_yScreen)
		nY2 = m_yScreen;
	nWidth = nX2 - nX;
	nHeight = nY2 - nY;
	// 创建一个与屏幕设备描述表兼容的位图
	hBitmap = CreateCompatibleBitmap
	(hScrDC, nWidth, nHeight);
	// 把新位图选到内存设备描述表中
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	// 把屏幕设备描述表拷贝到内存设备描述表中
	if (bSave)
	{
		//创建兼容DC,当bSave为中时把开始保存的全屏位图,按截取矩形大小保存
		CDC dcCompatible;
		dcCompatible.CreateCompatibleDC(CDC::FromHandle(hMemDC));
		dcCompatible.SelectObject(m_pBitmap);
		
		BitBlt(hMemDC, 0, 0, nWidth, nHeight,
			dcCompatible, nX, nY, SRCCOPY);
	}
	else
	{
		BitBlt(hMemDC, 0, 0, nWidth, nHeight,
			hScrDC, nX, nY, SRCCOPY);
	}

	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

	//得到屏幕位图的句柄
	//清除 

	if (bSave)
	{
		if (OpenClipboard())
		{
			//清空剪贴板
			EmptyClipboard();
			//把屏幕内容粘贴到剪贴板上,
			//hBitmap 为刚才的屏幕位图句柄
			SetClipboardData(CF_BITMAP, hBitmap);
			//关闭剪贴板
			CloseClipboard();
		}
		Mat img = bmp2mat(hScrDC, hBitmap);
		//imwrite("image.jpg", img);
		start_cognize(img, "symbols.txt");
	}

	DeleteDC(hScrDC);
	DeleteDC(hMemDC);
	// 返回位图句柄
	return hBitmap;
}

// 刷新局部窗口
void CCatchScreenDlg::InvalidateRgnWindow()
{
	//获取当全屏对话框窗口大小
	CRect rect1;
	GetWindowRect(rect1);

	CRgn rgn1;
	rgn1.CreateRectRgnIndirect(rect1);

	//获取更新区域,就是除了编辑框窗口不更新
	//m_rgn.CombineRgn(&rgn1, &rgn2, RGN_DIFF);

	// 添加ToolBar不刷新
	CRect rect2;
	::GetWindowRect(m_toolBar.GetHWND(), &rect2);
	CRgn rgn2;
	rgn2.CreateRectRgnIndirect(rect2);

	m_rgn.CombineRgn(&rgn1, &rgn2, RGN_DIFF);

	InvalidateRgn(&m_rgn);
}

BOOL CCatchScreenDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	bool bHandle = true;
	HWND hWnd = m_toolBar.GetHWND();
	if (lParam == (LPARAM)m_toolBar.GetHWND())
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case MyToolBar_ID:
			//PostQuitMessage(0);
			EndDialog(0);
			break;
		case MyToolBar_ID + 1: // finish button
			CopyScreenToBitmap(m_rectTracker.m_rect, TRUE);
			//PostQuitMessage(0);
			EndDialog(0);
			break;
		default:
			bHandle = false;
			break;
		}
		::SetFocus(hWnd);
	}
	if (bHandle == false)
	{
		return CDialog::OnCommand(wParam, lParam);
	}
}

////////////////////////////////// END OF FILE ///////////////////////////////////////