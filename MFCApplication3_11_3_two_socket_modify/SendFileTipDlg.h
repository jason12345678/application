#pragma once

#include "resource.h"
// CSendFileTipDlg 对话框

class CSendFileTipDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSendFileTipDlg)

public:
	CSendFileTipDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSendFileTipDlg();

// 对话框数据
	enum { IDD = IDD_SAVE_FILE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedSaveas();
	afx_msg void OnBnClickedCancel();
};
