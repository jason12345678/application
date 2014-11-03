// asav_sampleDlg.h : header file
//

#pragma once
#include <smartnet.h>
#include "ListCtrlExt.h" 
#include "TlHelp32.h"
#include "TCHAR.H"
#define INTERVAL 2
#define MAX_PROCESS_COUNT 200
#define MAX_PRIORITY_LEVEL 10

typedef struct _PROCESS
{
	TCHAR  proc_name[MAX_PATH];
	DWORD  pid;
	UINT32 tx_prio;
	UINT32 rx_prio;
	UINT32 tx_bw;
	UINT32 rx_bw;
	TCHAR  info[MAX_PATH];
}PROCESS;

// Casav_sampleDlg dialog
class Casav_sampleDlg : public CDialog
{
// Construction
public:
	Casav_sampleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ASAV_SAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
    
	CListBox		m_list_left;
	CListCtrl		m_list_right;

	RECT			m_rect, m_rect_s;
	smart_net       *m_smart_net;
	CList<PROCESS, PROCESS&> m_process_left, m_process_right;
	CTabCtrl		m_tab;
	int				m_tab_sel;
	CListCtrlExt	m_list_bw;
	CComboBox       m_Combo;
	CFont*          m_pFont;
	CImageList      m_imgList;
	bool TimerLock;

	void UpdateProcess(BOOL pid_changed, BOOL net_changed);
	void UpdateListBw(PROCESS& process, int i);
	void ConvertBpsToStr(DWORD Bps, TCHAR *str, int size);
	void InsertToListRight(int i, PROCESS& process);
	void GetPrioDes(ULONG prio, TCHAR *str, int size);
	void UpdateBandwidth();
	void UpdateAllItems();
	void PopulateList();
	void AutoPrioSet();
	void OnClose();
	static BOOL EndEditor(CWnd** pWnd, int nRow, int nColumn, CString &strSubItemText, DWORD_PTR dwItemData, void* pThis, BOOL bUpdate);
	static BOOL InitEditor(CWnd** pWnd, int nRow, int nColumn, CString &strSubItemText, DWORD_PTR dwItemData, void* pThis, BOOL bUpdate);
	
	enum {
        LIST_PROC_NAME,
        LIST_TX_PRIO,
        LIST_RX_PRIO
    };

	enum {
        LIST_PID,
		LIST_COUNT,
        LIST_PROCESS,
		LIST_NAME,
        LIST_TX_BW,
        LIST_RX_BW,
		LIST_PRIO
    };

	enum {
		ULTRAHIGH = 1,
		HIGH = 2,
		NORMAL = 3,
		LOW = 4
	};

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnDel();
public:
	afx_msg void OnBnClickedBtnUp();
	afx_msg void OnBnClickedBtnUpToTop();
public:
	afx_msg void OnBnClickedBtnDown();
	afx_msg void OnBnClickedBtnDownToBottom();
public:
	afx_msg void OnBnClickedBtnApply();
	afx_msg void OnBnClickedBtnSet();
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnBnClickedBtnRefresh();

	
public:
	int EnumProcess(PROCESS * ProcessList, long MaxSize);
	void RefreshListBw(PROCESS* process, int count);
	void GetProcessName(LPTSTR name,DWORD processid,LPTSTR buf,int len);
};
