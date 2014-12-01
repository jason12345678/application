// SmartSharingDlg1.cpp : 实现文件
//

#include "stdafx.h"
#include "SmartSharingDlg1.h"
#include "afxdialogex.h"
#include "CeBtnST.h"

// SmartSharingDlg1 对话框
IMPLEMENT_DYNAMIC(SmartSharingDlg1, CDialog)

SmartSharingDlg1::SmartSharingDlg1(CWnd* pParent /*=NULL*/)
	: CDialog(SmartSharingDlg1::IDD, pParent)
{

}

SmartSharingDlg1::~SmartSharingDlg1()
{
}

void SmartSharingDlg1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_button1);
	DDX_Control(pDX, IDC_BUTTON2, m_button2);
	DDX_Control(pDX, IDC_BUTTON3, m_button3);
	DDX_Control(pDX, IDC_BUTTON4, m_button4);
}


BEGIN_MESSAGE_MAP(SmartSharingDlg1, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &SmartSharingDlg1::OnBnClickedButton1)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON2, &SmartSharingDlg1::OnBnClickedButton2)
	ON_COMMAND(ID_SCANPEERS, &SmartSharingDlg1::OnScanpeers)
	ON_COMMAND(ID_START_BAND_WIDTH, &SmartSharingDlg1::OnStartBandWidth)
	ON_COMMAND(ID_STARTSERVICE, &SmartSharingDlg1::OnStartservice)
	ON_COMMAND(ID_STOPSERVICE, &SmartSharingDlg1::OnStopservice)
	ON_BN_CLICKED(IDC_BUTTON3, &SmartSharingDlg1::OnBnClickedButton3)
	ON_BN_CLICKED(DC_STOP_SRV, &SmartSharingDlg1::OnBnClickedStopSrv)
	ON_BN_CLICKED(IDC_BTNSCAN, &SmartSharingDlg1::OnBnClickedBtnscan)
	ON_BN_CLICKED(IDC_RESTART, &SmartSharingDlg1::OnBnClickedRestart)
END_MESSAGE_MAP()


// SmartSharingDlg1 消息处理程序


void SmartSharingDlg1::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
}


BOOL SmartSharingDlg1::OnInitDialog()
{


    COLORREF	crBtnColor = 200;
	CDialog::OnInitDialog();
    m_button1.SetFont(GetFont());
	m_button1.SetIcon(IDI_ICON1, CSize(32, 32), IDI_ICON1, CSize(32, 32));
	m_button1.SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
	m_button1.SetAlign(CCeButtonST::ST_ALIGN_VERT);
	m_button1.SetMenu(IDR_MENU3, m_hWnd);

	m_button2.SetFont(GetFont());
	m_button2.SetIcon(IDI_ICON2, CSize(32, 32), IDI_ICON2, CSize(32, 32));
	m_button2.SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
	m_button2.SetAlign(CCeButtonST::ST_ALIGN_VERT);
	m_button2.SetMenu(IDR_MENU4, m_hWnd);

	m_button3.SetFont(GetFont());
	m_button3.SetIcon(IDI_ICON3, CSize(32, 32), IDI_ICON3, CSize(32, 32));
	m_button3.SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
	m_button3.SetAlign(CCeButtonST::ST_ALIGN_VERT);
	m_button3.SetMenu(IDR_MENU5, m_hWnd);

	m_button4.SetFont(GetFont());
	m_button4.SetIcon(IDI_ICON4, CSize(32, 32), IDI_ICON4, CSize(32, 32));
	m_button4.SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
	m_button4.SetAlign(CCeButtonST::ST_ALIGN_VERT);
	m_button4.SetMenu(IDR_MENU6, m_hWnd);

	return TRUE;  // return TRUE unless you set the focus to a control

}


void SmartSharingDlg1::OnPaint()
{
	CPaintDC dc(this); // device context for painting


		// TODO: 在此处添加消息处理程序代码
		// 不为绘图消息调用 CDialog::OnPaint()
			CBitmap bitmap;   
	bitmap.LoadBitmap(IDB_BITMAP1);
	CBrush brush; 
	brush.CreatePatternBrush(&bitmap);  
	CBrush* pOldBrush = dc.SelectObject(&brush);  
	dc.Rectangle(0,0,900,800);
}


void SmartSharingDlg1::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
}



void SmartSharingDlg1::OnScanpeers()
{
}


void SmartSharingDlg1::OnStartBandWidth()
{
	// TODO: 在此添加命令处理程序代码
	HWND hPreWnd;
    if (hPreWnd = ::FindWindow(NULL, _T("BandWidthManager")))
    {
		/*
        if (IsIconic(hPreWnd))  // IsIconic: Is Minisized? IsZoomed: Is Maxisized?
         {
            ::ShowWindow( hPreWnd, SW_SHOWNORMAL );
            theApp.SetForegroundWindow(hPreWnd);
        }
		*/
		AfxMessageBox(_T("asav has exist."));
		return;
    }
	ShellExecute(NULL, L"open", L"BandWidthManager.exe", L"", L"", SW_HIDE);
}


void SmartSharingDlg1::OnStartservice()
{
	// TODO: 在此添加命令处理程序代码
}


void SmartSharingDlg1::OnStopservice()
{
	// TODO: 在此添加命令处理程序代码
}


void SmartSharingDlg1::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
}


void SmartSharingDlg1::OnBnClickedStopSrv()
{
	// TODO: Add your control notification handler code here
}


void SmartSharingDlg1::OnBnClickedBtnscan()
{
	// TODO: Add your control notification handler code here
}


void SmartSharingDlg1::OnBnClickedRestart()
{
	// TODO: Add your control notification handler code here
}
