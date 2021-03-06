
// SmartSharingDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "LocalState.h"
#include "CeBtnST.h"
#include "MyDataAsyncSocket.h"
#include "CoAsyncSocket.h"
#include "MyDataAsyncSocket.h"
#include "ServerDataAsyncSocket.h"
#include "resource.h"

#define WM_TEST WM_USER+101 //用户消息

typedef struct _peerinform{ 

	CString SSID;
	CString Status;
	CString FileSrv;
	CString ICSSrv; 
	CString DSSrv;
}peerinform;

typedef struct _buttonlist{

	CString	cSSID ;
	CString cIP;
}buttonlist;

#define SIZE 5
// SmartSharingDlg 对话框
class SmartSharingDlg : public CDialog
{
// 构造
public:
	SmartSharingDlg(CWnd* pParent = NULL);	// 标准构造函数
	~SmartSharingDlg();
	static DWORD WINAPI SendFile(_In_  LPVOID lpParameter);
	static DWORD WINAPI ServerSendFile(_In_  LPVOID lpParameter);
	static DWORD WINAPI DisConnectClient(LPVOID p);
	static DWORD WINAPI handleWlanEvent(_In_  LPVOID lpParameter);
	static DWORD WINAPI handleCheckNetworkLiveEvent(_In_  LPVOID lpParameter);
	BOOL SendFileContent2Net(char *, int, UINT, LPCTSTR, unsigned int , unsigned int);
    BOOL SendFileContent2Net1(char *, int, UINT, LPCTSTR, unsigned int , unsigned int);
	BOOL isConnected();
	BOOL hasService(int index);
	static DWORD WINAPI connect2Ap(_In_  LPVOID lpParameter);
	CString GetConnectedSSID();
	void addClient(CString mac);
	void delClient(CString mac);

// 对话框数据
	enum { IDD = IDD_MFCAPPLICATION3_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	
public:

	int m_Index; 
	CRITICAL_SECTION m_peerSection;
	bool is_dragfile;
	CString drag_filepath;
	CRect m_rect;
	bool is_send_file_flag;
	bool is_send_file_client_cancel;
	bool is_send_file_server_cancel;
	int flag_show;
	TCHAR mfile_flag;
	int m_Indexclient;
	bool m_Indexclientflag;
	bool m_networkrunning;


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	void OnClose();
	void changeSize(int ID, int cx, int cy);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnscan();
	BOOL PreTranslateMessage(MSG* pMsg);

private:
	static DWORD WINAPI ScanAP(_In_  LPVOID lpParameter);
	static DWORD WINAPI HandleMessage(_In_  LPVOID lpParameter);
	static std::vector<WLAN_BSS_ENTRY> vecSelectedBSSEntry;
	HANDLE hThreadScan;
	CMyAsyncSocket rcvSock;
	CCoAsyncSocket rcvServerSock;
	CMyDataAsyncSocket rcvdataSock;
	CServerDataAsyncSocket rcvserverdataSock;

	CAsyncSocket sndSock;
	int connectedItem;
	BOOL handleWlanEventFlag;
	peerinform peerinformList[SIZE];
	buttonlist buttonList[SIZE];
private:
	void clearpeerList();
	void clearclientList();
	void clearbuttonlist();
	void clearcontrolbox();
	void clearctlprocess();
	void setconnectImage(int index);
	int  CheckTransferStaus();
public:

	CProgressCtrl m_progress;
	CString ip_gateway;
	CString dst_ip;
	BOOL toClientFlag;
	afx_msg void OnBnClickedBandwidthBtn();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMenuConnect();
	afx_msg void OnMenuDisconnect();
	afx_msg void OnMenuSendfile();
	afx_msg void OnLbnSelchangeListPeer();
	CListCtrl m_listCont;
	CButton m_fileSrvFlag;
	CButton m_ICSSrvFlag;
	CButton m_DSSrvFlag;
	afx_msg void OnBnClickedFileChkbox();
	afx_msg void OnBnClickedIcsChkbox();
	afx_msg void OnBnClickedDsChkbox();
	afx_msg void OnBnClickedRestart();
	CListCtrl m_listConn;
	afx_msg void OnListSendfile();

    CLocalState m_LocalState;
	afx_msg void OnBnClickedStopSrv();
	CCeButtonST m_button1;
	CCeButtonST m_button2;
	CCeButtonST m_button3;
	CCeButtonST m_button4;
	afx_msg void OnScanpeers();
	afx_msg void OnStartBandWidth();
	afx_msg void OnStartservice();
	afx_msg void OnStopservice();

	
	CCeButtonST m_peerbtn6;
	CCeButtonST m_peerbtn7;
	CCeButtonST m_peerbtn8;
	CCeButtonST m_peerbtn9;
	CCeButtonST m_peerbtn10;
	CCeButtonST m_peerbtn11;
	CCeButtonST m_peerbtn12;
	CCeButtonST m_peerbtn13;
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton10();
	afx_msg void OnBnClickedButton11();
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedButton13();
	afx_msg LRESULT  OnTest(WPARAM wParam,LPARAM lParam);
	CCeButtonST m_peerbtn14;
	CCeButtonST m_peerbtn15;
	afx_msg void OnBnClickedButton14();
	afx_msg void OnBnClickedButton15();
	CCeButtonST m_peerclientbtn1;
	CCeButtonST m_peerclientbtn2;
	CCeButtonST m_peerclientbtn3;
	CCeButtonST m_peerclientbtn4;
	CCeButtonST m_peerclientbtn5;
	CCeButtonST m_peerclientbtn6;
	CCeButtonST m_peerclientbtn7;
	CCeButtonST m_peerclientbtn8;
	CCeButtonST m_peerclientbtn9;
	CCeButtonST m_peerclientbtn10;
	afx_msg void OnBnClickedButton16();
	afx_msg void OnBnClickedButton17();
	afx_msg void OnBnClickedButton18();
	afx_msg void OnBnClickedButton19();
	afx_msg void OnBnClickedButton20();
	afx_msg void OnBnClickedButton21();
	afx_msg void OnBnClickedButton22();
	afx_msg void OnBnClickedButton23();
	afx_msg void OnBnClickedButton24();
	afx_msg void OnBnClickedButton25();

	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg void OnListDisconnect();
	afx_msg void OnBnClickedButton4();

	void SmartSharingDlg::DisConnect(int index);
	
//	CStatic m_picture;
	//CPicture m_picture;
	afx_msg void OnStnClickedPicture();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnLvnItemchangedListCont(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuSendfilecancel();
	afx_msg void OnListSendfilecancel();
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
private:
	BOOL CloseProcess(LPCSTR lpstrName);
	void OnScreenSharing();
	CString GetExePath();

};
extern SmartSharingDlg *mainDlg;
