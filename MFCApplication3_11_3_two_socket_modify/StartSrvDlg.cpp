// StartSrvDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "StartSrvDlg.h"
#include "afxdialogex.h"


// CStartSrvDlg 对话框

IMPLEMENT_DYNAMIC(CStartSrvDlg, CDialogEx)

CStartSrvDlg::CStartSrvDlg(CWnd* pParent, int *fileFlag, int *icsFlag, int *dsFlag)
	: CDialogEx(CStartSrvDlg::IDD, pParent)
{
	file_flag = fileFlag;
	ics_flag = icsFlag;
	ds_flag = dsFlag;
}

CStartSrvDlg::~CStartSrvDlg()
{
}

void CStartSrvDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_FLAG, m_fileFlag);
	DDX_Control(pDX, IDC_ICS_FLAG, m_icsFlag);
	DDX_Control(pDX, IDC_DS_FLAG, m_dsFlag);
}


BEGIN_MESSAGE_MAP(CStartSrvDlg, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &CStartSrvDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CStartSrvDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CStartSrvDlg 消息处理程序


void CStartSrvDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


void CStartSrvDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	*file_flag = m_fileFlag.GetCheck();
	*ics_flag = m_icsFlag.GetCheck();
	*ds_flag = m_dsFlag.GetCheck();
	CDialogEx::OnOK();
}
