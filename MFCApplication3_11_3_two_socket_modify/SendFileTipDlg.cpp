// SendFileTipDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SendFileTipDlg.h"
#include "afxdialogex.h"


// CSendFileTipDlg 对话框

IMPLEMENT_DYNAMIC(CSendFileTipDlg, CDialogEx)

CSendFileTipDlg::CSendFileTipDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSendFileTipDlg::IDD, pParent)
{

}
BOOL CSendFileTipDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;
}


CSendFileTipDlg::~CSendFileTipDlg()
{
}

void CSendFileTipDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSendFileTipDlg, CDialogEx)
	ON_BN_CLICKED(IDSAVE, &CSendFileTipDlg::OnBnClickedSave)
	ON_BN_CLICKED(IDSAVEAS, &CSendFileTipDlg::OnBnClickedSaveas)
	ON_BN_CLICKED(IDCANCEL, &CSendFileTipDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CSendFileTipDlg 消息处理程序


void CSendFileTipDlg::OnBnClickedSave()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
}


void CSendFileTipDlg::OnBnClickedSaveas()
{
	CDialogEx::OnOK();
}


void CSendFileTipDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}
