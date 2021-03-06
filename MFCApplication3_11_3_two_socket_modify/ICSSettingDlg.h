#pragma once

#include "resource.h"
#include "afxwin.h"
// CICSSettingDlg 对话框

class CICSSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CICSSettingDlg)

public:
	CICSSettingDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CICSSettingDlg();
	void ICSConfigure();

// 对话框数据
	enum { IDD = IDD_ICS_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_cbInterface;
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeAllConnectInterface();
};
