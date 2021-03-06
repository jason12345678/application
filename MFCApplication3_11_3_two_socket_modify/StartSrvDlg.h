#pragma once

#include "resource.h"
#include "afxwin.h"
// CStartSrvDlg 对话框

class CStartSrvDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStartSrvDlg)

public:
	CStartSrvDlg(CWnd* pParent = NULL, int *fileFlag = 0, int *icsFlag = 0, int *dsFlag = 0);   // 标准构造函数
	virtual ~CStartSrvDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG1 };

private:
	int *file_flag;
	int *ics_flag;
	int *ds_flag;
	int flag_show;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	CButton m_fileFlag;
	CButton m_icsFlag;
	CButton m_dsFlag;
};
