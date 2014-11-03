#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "LocalState.h"
#include "Resource.h"
#include "CeBtnST.h"


// SmartSharingDlg1 对话框

class SmartSharingDlg1 : public CDialog
{
	DECLARE_DYNAMIC(SmartSharingDlg1)

public:
	SmartSharingDlg1(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~SmartSharingDlg1();

// 对话框数据
	enum { IDD = IDD_MFCAPPLICATION3_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CCeButtonST m_button1;
    CCeButtonST m_button2;
	CCeButtonST m_button3;
	CCeButtonST m_button4;
	afx_msg void OnBnClickedButton1();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();

	afx_msg void OnBnClickedButton2();
	afx_msg void OnScanpeers();
	afx_msg void OnStartBandWidth();
	afx_msg void OnStartservice();
	afx_msg void OnStopservice();
	CLocalState m_LocalState;
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedStopSrv();
};
