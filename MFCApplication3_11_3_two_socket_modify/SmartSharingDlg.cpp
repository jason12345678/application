
// SmartSharingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SmartSharingApp.h"
#include "SmartSharingDlg.h"
#include "afxdialogex.h"
#include "ICSSettingDlg.h"
#include "CeBtnST.h"
#include "md5_check.h"
#include <map>
#include "log.h"
#include <wininet.h>
#pragma comment(lib,"Wininet.lib")
using namespace std;





#ifdef _DEBUGG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
std::vector<WLAN_BSS_ENTRY> SmartSharingDlg::vecSelectedBSSEntry;

extern map<CString ,client_data *  >myMap;

 int g_bDebug=0;

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClickedOk();

	afx_msg void OnListSendfile();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)


END_MESSAGE_MAP()


// SmartSharingDlg 对话框

HANDLE hMutexFileCont;
HANDLE hMutexFileStart;
HANDLE hMutexFileEnd;
HANDLE hMutexDisconnect;
HANDLE hMutexConnectToServer;
SmartSharingDlg::SmartSharingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SmartSharingDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	if(!rcvSock.Create(SOCK_PORT_RECV, SOCK_DGRAM, FD_READ | FD_WRITE/*, _T("192.168.137.1")*/))
	{
		AfxMessageBox (_T("network error"));
		trace("initialize  socket failure. ");
		return ;
	}
	if(!rcvdataSock.Create(SOCK_PORT_RECV_DATA, SOCK_DGRAM, FD_READ | FD_WRITE/*, _T("192.168.137.1")*/))
	{
		AfxMessageBox (_T("network error"));
		trace("initialize  socket failure .");
		return;
	}
	
	if(!rcvServerSock.Create(SOCK_PORT_RECV_SERVER, SOCK_DGRAM, FD_READ | FD_WRITE))
	{
		AfxMessageBox (_T("network error"));
		trace("initialize  socket failure .");
		return;
	}

	if(!rcvserverdataSock.Create(SOCK_PORT_RECV_SERVER_DATA, SOCK_DGRAM, FD_READ | FD_WRITE))
	{
		AfxMessageBox (_T("network error"));
		trace("initialize  socket failure.");
		return;
	}

	
	//sndSock.m_hSocket = INVALID_SOCKET;	
	// hMutexSendFile = CreateMutex(NULL, TRUE, NULL);
	hMutexFileStart = CreateEvent(NULL, FALSE, FALSE, NULL);
	hMutexFileCont = CreateEvent(NULL, FALSE, FALSE, NULL);
	hMutexFileEnd = CreateEvent(NULL, FALSE, FALSE, NULL);
	hMutexConnectToServer= CreateEvent(NULL, FALSE, FALSE, NULL);
	hMutexDisconnect = CreateEvent(NULL, FALSE, FALSE, NULL);
	ackPkg = 0;
	g_md5_is_true = 0;
	toClientFlag = FALSE;
//===================================================================

}

SmartSharingDlg::~SmartSharingDlg()
{
	CloseProcess("taskkill /im app_refs_ut_capserver.exe /f");
	CloseProcess("taskkill /im app_refs_ut_simpleplayer.exe /f");
}

void SmartSharingDlg::OnClose()
{
	if ((m_LocalState.GetCurrentState() == CLocalState::SENDFILEACKOK)
		|| (m_LocalState.GetCurrentState() == CLocalState::SENDFILEWAITACK))
	{
		AfxMessageBox(_T("You are sending files, please don't exit the window!"));
		return;
	}
	if (m_LocalState.GetCurrentState() == CLocalState::RECEIVINGFILE)
	{
		AfxMessageBox(_T("You are receiving files, please don't exit the window!"));
		return;
	}
	/*
	if(CheckTransferStaus()){
		AfxMessageBox(_T("You are sending files, please don't exit the window!"));
		return;
	}
	*/
	handleWlanEventFlag = FALSE;
	EnterCriticalSection(&m_PeerChangeCond);
	wlanEvtType = WLAN_EVT_INVAILD;
	LeaveCriticalSection(&m_PeerChangeCond);
	SetEvent(hPeerChangeEvent);

	 map<CString ,client_data *  >myMap;
	 for( map<CString ,client_data *  >::iterator i=myMap.begin(); i!=myMap.end(); i++)  
    {  
		  client_data *p = i->second;
		  delete p;
          myMap.erase(i);  
       
    }
	
	//int rst;
	//rst = CWfdWlanSolution::Instance()->DisConnectToAP();
	CWfdWlanSolution::Instance()->HostedNetworkStopSoftAp();
	CWfdWlanSolution::Instance()->UnregisterNotificationFunc();
	CDialog::OnClose();
}

void SmartSharingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_LIST_CONT, m_listCont);
	DDX_Control(pDX, IDC_FILE_CHKBOX, m_fileSrvFlag);
	DDX_Control(pDX, IDC_ICS_CHKBOX, m_ICSSrvFlag);
	DDX_Control(pDX, IDC_DS_CHKBOX, m_DSSrvFlag);
	DDX_Control(pDX, IDC_LIST_CONN, m_listConn);
	DDX_Control(pDX, IDC_BUTTON1, m_button1);
	DDX_Control(pDX, IDC_BUTTON2, m_button2);
	DDX_Control(pDX, IDC_BUTTON3, m_button3);
	DDX_Control(pDX, IDC_BUTTON4, m_button4);
	DDX_Control(pDX, IDC_BUTTON6, m_peerbtn6);
	DDX_Control(pDX, IDC_BUTTON7, m_peerbtn7);
	DDX_Control(pDX, IDC_BUTTON8, m_peerbtn8);
	DDX_Control(pDX, IDC_BUTTON9, m_peerbtn9);
	DDX_Control(pDX, IDC_BUTTON10, m_peerbtn10);
	DDX_Control(pDX, IDC_BUTTON11, m_peerbtn11);
	DDX_Control(pDX, IDC_BUTTON12, m_peerbtn12);
	DDX_Control(pDX, IDC_BUTTON13, m_peerbtn13);
	DDX_Control(pDX, IDC_BUTTON14, m_peerbtn14);
	DDX_Control(pDX, IDC_BUTTON15, m_peerbtn15);
	DDX_Control(pDX, IDC_BUTTON16, m_peerclientbtn1);
	DDX_Control(pDX, IDC_BUTTON17, m_peerclientbtn2);
	DDX_Control(pDX, IDC_BUTTON18, m_peerclientbtn3);
	DDX_Control(pDX, IDC_BUTTON19, m_peerclientbtn4);
	DDX_Control(pDX, IDC_BUTTON20, m_peerclientbtn5);
	DDX_Control(pDX, IDC_BUTTON21, m_peerclientbtn6);
	DDX_Control(pDX, IDC_BUTTON22, m_peerclientbtn7);
	DDX_Control(pDX, IDC_BUTTON23, m_peerclientbtn8);
	DDX_Control(pDX, IDC_BUTTON24, m_peerclientbtn9);
	DDX_Control(pDX, IDC_BUTTON25, m_peerclientbtn10);
	
	//  DDX_Control(pDX, IDC_PICTURE, m_picture);

}

BEGIN_MESSAGE_MAP(SmartSharingDlg, CDialog)
    ON_MESSAGE(WM_TEST,OnTest)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTNSCAN, &SmartSharingDlg::OnBnClickedBtnscan)
	//ON_BN_CLICKED(IDC_SEND_FILE_BTN, &SmartSharingDlg::OnBnClickedSendFileBtn)
	ON_BN_CLICKED(IDC_BANDWIDTH_BTN, &SmartSharingDlg::OnBnClickedBandwidthBtn)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_MENU_CONNECT, &SmartSharingDlg::OnMenuConnect)
	ON_COMMAND(ID_MENU_DISCONNECT, &SmartSharingDlg::OnMenuDisconnect)
	ON_COMMAND(ID_MENU_SENDFILE, &SmartSharingDlg::OnMenuSendfile)
	ON_LBN_SELCHANGE(IDC_LIST_PEER, &SmartSharingDlg::OnLbnSelchangeListPeer)
	ON_BN_CLICKED(IDC_FILE_CHKBOX, &SmartSharingDlg::OnBnClickedFileChkbox)
	ON_BN_CLICKED(IDC_ICS_CHKBOX, &SmartSharingDlg::OnBnClickedIcsChkbox)
	ON_BN_CLICKED(IDC_DS_CHKBOX, &SmartSharingDlg::OnBnClickedDsChkbox)
	ON_BN_CLICKED(IDC_RESTART, &SmartSharingDlg::OnBnClickedRestart)
	ON_COMMAND(ID_LIST_SENDFILE, &SmartSharingDlg::OnListSendfile)
	ON_BN_CLICKED(DC_STOP_SRV, &SmartSharingDlg::OnBnClickedStopSrv)
	ON_COMMAND(ID_SCANPEERS, &SmartSharingDlg::OnScanpeers)
	ON_COMMAND(ID_START_BAND_WIDTH, &SmartSharingDlg::OnStartBandWidth)
	ON_COMMAND(ID_STARTSERVICE, &SmartSharingDlg::OnStartservice)
	ON_COMMAND(ID_STOPSERVICE, &SmartSharingDlg::OnStopservice)
	ON_BN_CLICKED(IDC_BUTTON6, &SmartSharingDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &SmartSharingDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &SmartSharingDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &SmartSharingDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, &SmartSharingDlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON11, &SmartSharingDlg::OnBnClickedButton11)
	ON_BN_CLICKED(IDC_BUTTON12, &SmartSharingDlg::OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON13, &SmartSharingDlg::OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON14, &SmartSharingDlg::OnBnClickedButton14)
	ON_BN_CLICKED(IDC_BUTTON15, &SmartSharingDlg::OnBnClickedButton15)
	ON_BN_CLICKED(IDC_BUTTON16, &SmartSharingDlg::OnBnClickedButton16)
	ON_BN_CLICKED(IDC_BUTTON17, &SmartSharingDlg::OnBnClickedButton17)
	ON_BN_CLICKED(IDC_BUTTON18, &SmartSharingDlg::OnBnClickedButton18)
	ON_BN_CLICKED(IDC_BUTTON19, &SmartSharingDlg::OnBnClickedButton19)
	ON_BN_CLICKED(IDC_BUTTON20, &SmartSharingDlg::OnBnClickedButton20)
	ON_BN_CLICKED(IDC_BUTTON21, &SmartSharingDlg::OnBnClickedButton21)
	ON_BN_CLICKED(IDC_BUTTON22, &SmartSharingDlg::OnBnClickedButton22)
	ON_BN_CLICKED(IDC_BUTTON23, &SmartSharingDlg::OnBnClickedButton23)
	ON_BN_CLICKED(IDC_BUTTON24, &SmartSharingDlg::OnBnClickedButton24)
	ON_BN_CLICKED(IDC_BUTTON25, &SmartSharingDlg::OnBnClickedButton25)
	ON_WM_DROPFILES()
	ON_WM_SIZE()
	ON_COMMAND(ID_LIST_DISCONNECT, &SmartSharingDlg::OnListDisconnect)
	ON_BN_CLICKED(IDC_BUTTON4, &SmartSharingDlg::OnBnClickedButton4)
	
	ON_STN_CLICKED(IDC_PICTURE, &SmartSharingDlg::OnStnClickedPicture)
	ON_BN_CLICKED(IDC_BUTTON3, &SmartSharingDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON1, &SmartSharingDlg::OnBnClickedButton1)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CONT, &SmartSharingDlg::OnLvnItemchangedListCont)
	ON_COMMAND(ID_MENU_SENDFILECANCEL, &SmartSharingDlg::OnMenuSendfilecancel)
	ON_COMMAND(ID_LIST_SENDFILECANCEL, &SmartSharingDlg::OnListSendfilecancel)
	ON_COMMAND(ID_LIST_SCREENSHARING, &SmartSharingDlg::OnScreenSharing)
END_MESSAGE_MAP()
    


// SmartSharingDlg 消息处理程序

enum STATE {DISCONNECT=0, CONNECTED,SERVICERUN};


LRESULT  SmartSharingDlg::OnTest(WPARAM wParam, LPARAM lParam)
//从lParam中取出CString对象的指针,并将字符串内容在IDC_MSGEDIT中显示出来
{

    int *p;
    p=(int *)lParam;
	if(*p ==1){


		COLORREF	crBtnColor = 200;
		CCeButtonST *b =NULL;
		int *p = (int *)wParam;
		int index = *p;
		b =(CCeButtonST *)GetDlgItem(IDC_BUTTON6+index);
		b->ShowWindow(true);
		b->SetFont(GetFont());
		b->SetIcon(IDI_ICON5, CSize(32, 32), IDI_ICON5, CSize(32, 32));
    	b->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
		b->SetAlign(CCeButtonST::ST_ALIGN_VERT);
		b->SetMenu(IDR_MENU1, m_hWnd);
	///	m_Indexclient = index;
	//	m_Indexclientflag = false;

	}else if(*p ==111){


	  // if(m_LocalState.GetCurrentState() == DISCONNECT){

		


			COLORREF	crBtnColor = 200;
			CCeButtonST *b =NULL;
				int index = wParam;
			b =(CCeButtonST *)GetDlgItem(IDC_BUTTON6+index);
			b->ShowWindow(true);
			b->SetFont(GetFont());
			b->SetIcon(IDI_ICON6, CSize(32, 32), IDI_ICON6, CSize(32, 32));
    		b->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
			b->SetAlign(CCeButtonST::ST_ALIGN_VERT);
			b->SetMenu(IDR_MENU9, m_hWnd);
		//	GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T(" connecting  the server..."));
	//	}




	}
	else if(*p==2){
		trace("enter into disconnect p= 2");
		trace("m_LocalState.GetCurrentState()=%d",m_LocalState.GetCurrentState());
		if(m_LocalState.GetCurrentState() == CONNECTED){
			
			COLORREF	crBtnColor = 200;
			CCeButtonST *b =NULL;
			int *p = (int *)wParam;
			int index = *p;
			b =(CCeButtonST *)GetDlgItem(IDC_BUTTON6+index);
			b->ShowWindow(true);
			b->SetFont(GetFont());
			b->SetIcon(IDI_ICON5, CSize(32, 32), IDI_ICON5, CSize(32, 32));
    		b->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
			b->SetAlign(CCeButtonST::ST_ALIGN_VERT);
			if(mfile_flag != '0'){
				b->SetMenu(IDR_MENU1_1, m_hWnd);
			}else{

			    b->SetMenu(IDR_MENU1_2, m_hWnd);

			}
			m_Indexclient = index;
		    m_Indexclientflag = true;

			trace("leave  into  disconnect p= 2");

		}else if(m_LocalState.GetCurrentState() == DISCONNECT){

			COLORREF	crBtnColor = 200;
			CCeButtonST *b =NULL;
			int *p = (int *)wParam;
			int index = *p;
			b =(CCeButtonST *)GetDlgItem(IDC_BUTTON6+index);
			b->ShowWindow(true);
			b->SetFont(GetFont());
			b->SetIcon(IDI_ICON6, CSize(32, 32), IDI_ICON6, CSize(32, 32));
    		b->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
			b->SetAlign(CCeButtonST::ST_ALIGN_VERT);
			b->SetMenu(IDR_MENU1, m_hWnd);
			GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Disconnected."));
			m_Indexclient = index;
		    m_Indexclientflag = false;
		}


	}else if(*p ==3){

				int *p = (int *)wParam;
		
		    	int status = *p;
				int i;
				if(status==0){
					trace("enter connect the server status = 3");
					for(i=0;i<SIZE;i++){
						
						if(buttonList[i].cSSID!="" && buttonList[i].cIP!="")
						{

							trace("connect the server i=%d ",i);
								COLORREF	crBtnColor = 200;
								CCeButtonST *b =NULL;
								b =(CCeButtonST *)GetDlgItem(IDC_BUTTON16+i);
								if(!b->IsWindowVisible()){
									b->ShowWindow(true);
									b->SetFont(GetFont());
									b->SetIcon(IDI_ICON5, CSize(32, 32), IDI_ICON5, CSize(32, 32));
    								b->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
									b->SetAlign(CCeButtonST::ST_ALIGN_VERT);
									if(theApp.fileFlag){
										b->SetMenu(IDR_MENU2, m_hWnd);
									}else{
										b->SetMenu(IDR_MENU8, m_hWnd);
									}
									b->SetParent(this);
									b->SetServer(false);
									b->SetIndex(i);
									GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Service is Connected"));
									//OnBnClickedBtnscan();
								}
								
								
								b =(CCeButtonST *)GetDlgItem(IDC_BUTTON6+i);
								b->ShowWindow(false);

						}
					}
					trace("leave  connect  the server status = 3");

				}else if(status==1){
						CProgressCtrl *progress;
						for(i=0;i<SIZE;i++){
						
						if(buttonList[i].cSSID=="" && buttonList[i].cIP=="")
						{
								COLORREF	crBtnColor = 200;
								CCeButtonST *b =NULL;
								b =(CCeButtonST *)GetDlgItem(IDC_BUTTON16+i);
								if(b->IsWindowVisible()){
									b->ShowWindow(false);
								}

								progress =(CProgressCtrl *)GetDlgItem(IDC_PROGRESS1+i);
								progress->SetPos(0);
								if(progress->IsWindowVisible()){
									progress->ShowWindow(false);
									GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("file transfering is  interrupted."));
									m_LocalState.GotoReceiveFileCancel();
									Sleep(1000);
									GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Disconnected"));
								}
							
						}
					}

				}

	}else if (*p ==4){

		COLORREF	crBtnColor = 200;
		CCeButtonST *b =NULL;
		int i=0;
		for( i=0;i<10;i++){
			b =(CCeButtonST *)GetDlgItem(IDC_BUTTON6+i);
		    if(b->IsWindowVisible()){
			b->SetFont(GetFont());
			b->SetIcon(IDI_ICON6, CSize(32, 32), IDI_ICON6, CSize(32, 32));
    		b->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
			b->SetAlign(CCeButtonST::ST_ALIGN_VERT);
			b->SetMenu(IDR_MENU1, m_hWnd);
			GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Disconnected."));
			m_Indexclient = i;
		    m_Indexclientflag = false;
			}

		}
		clearctlprocess();
	}else if(*p ==5) { ///接收进度条
		CProgressCtrl *progress;
		int index = 0;
		int percent = 0;
		message *msg =(message *) wParam;
		index = msg->index;
		percent = msg->percent; 
		progress =(CProgressCtrl *)GetDlgItem(IDC_PROGRESS1+index);
		progress->SetPos(percent);	
		if(percent==0){
				progress->ShowWindow(false);
		}else{
				progress->ShowWindow(true);
		}
	}else if(*p ==6){  ///接收开始进度条

		CProgressCtrl *progress;
		int index = 0;
		int percent = 0;
		message *msg =(message *) wParam;
		index = msg->index;
		percent = msg->percent; 
		progress =(CProgressCtrl *)GetDlgItem(IDC_PROGRESS1+index);
		progress->SetPos(percent);	
		progress->ShowWindow(true);

	}else if(*p ==7){  ///发送开始进度条可见

		CProgressCtrl *progress;
		int index = 0;
		int percent = 0;
		message *msg =(message *) wParam;
		index = msg->index;
		percent = msg->percent; 
		progress =(CProgressCtrl *)GetDlgItem(IDC_PROGRESS1+index);
		progress->SetPos(percent);	
		progress->ShowWindow(true);
	}else if(*p ==8){  ///发送开始进度条不可见
		trace("enter into disconnect p= 8  progress");
		CProgressCtrl *progress;
		int index = 0;
		int percent = 0;
		message *msg =(message *) wParam;
		index = msg->index;
		percent = msg->percent; 
		progress =(CProgressCtrl *)GetDlgItem(IDC_PROGRESS1+index);
		progress->SetPos(percent);	
		progress->ShowWindow(false);
		trace("leave disconnect p= 8  progress");
	}else if (*p ==9){

		HANDLE h_sendFile = NULL;
		if(m_LocalState.GetCurrentState() == CONNECTED){
			
			dst_ip = ip_gateway;
			is_dragfile = true;
			dragmsg *msg = (dragmsg *) wParam;
			drag_filepath = msg->filepath;
			h_sendFile = CreateThread(NULL, 0, SendFile, this, 0, NULL);

		}
	//	AfxMessageBox(_T("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));

	}else if (*p ==10) {

		HANDLE h_sendFile = NULL;
		dragmsg *msg = (dragmsg *) wParam;
		//int *index =(int  *) wParam;
		dst_ip = buttonList[msg->index].cIP;
		toClientFlag = TRUE;
		is_dragfile = true;
		drag_filepath = msg->filepath;
		h_sendFile = CreateThread(NULL, 0, SendFile, this, 0, NULL);

	}else if (*p ==11){   /////receive the files;

		
		ULONGLONG *size = (ULONGLONG *) wParam;
		CString  s;
		s.Format(_T("Received file successed %lu bytes,"),*size);
		GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(s);
		//AfxMessageBox((LPCTSTR)s);

	}else if (*p ==12) {           ////


		CString  s;
		s.Format(_T("SendFile is running"));
		AfxMessageBox((LPCTSTR)s);
	}else if (*p ==13) {

		CString  s;
		s.Format(_T("Send file successed."));
		GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(s);
	}else if (*p ==14) {

	    CString  s;
		s.Format(_T("sending file please wait..."));
		GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(s);
	}else if (*p ==15) {

	    CString  s;
		s.Format(_T("receiving  file please wait..."));
		GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(s);
	}else if(*p ==16){

		COLORREF	crBtnColor = 200;
		CCeButtonST *b =NULL;
		int i;

		for(i=IDC_BUTTON6;i<=IDC_BUTTON15;i++){

			b =(CCeButtonST *)GetDlgItem(i);
			if(b->IsWindowVisible()){
				DisConnect(i-IDC_BUTTON6);
			//	b->ShowWindow(false);
				b->SetFont(GetFont());
				b->SetIcon(IDI_ICON5, CSize(32, 32), IDI_ICON5, CSize(32, 32));
    			b->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
				b->SetAlign(CCeButtonST::ST_ALIGN_VERT);
				b->SetMenu(IDR_MENU1, m_hWnd);
			}
			
		}


	}else if(*p ==17){  ///发送开始进度条可见  Server

		CProgressCtrl *progress;
		int index = 0;
		int percent = 0;
		message *msg =(message *) wParam;
		index = msg->index;
		percent = msg->percent; 
		progress =(CProgressCtrl *)GetDlgItem(IDC_PROGRESS6+index);
		progress->SetPos(percent);	
		progress->ShowWindow(true);
	}
	else if(*p ==18){  ///发送开始进度条不可见
		CProgressCtrl *progress;
		int index = 0;
		int percent = 0;
		message *msg =(message *) wParam;
		index = msg->index;
		percent = msg->percent; 
		progress =(CProgressCtrl *)GetDlgItem(IDC_PROGRESS6+index);
		progress->SetPos(percent);	
		progress->ShowWindow(false);
	}
	else if(*p==21){

		CProgressCtrl *progress;              
		int index = 0;
		int percent = 0;
		message *msg =(message *) wParam;
		index = msg->index;
		percent = msg->percent; 
		progress =(CProgressCtrl *)GetDlgItem(IDC_PROGRESS1+index);
		progress->SetPos(percent);	
		progress->ShowWindow(false);
		GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("file transfering is  canceled."));
	

	}
	else if(*p==22){

		for(int i=0;i<SIZE;i++){
		
			COLORREF	crBtnColor = 200;
			CCeButtonST *b =NULL;
			b =(CCeButtonST *)GetDlgItem(IDC_BUTTON16+i);
			if(b->IsWindowVisible()){
				b->ShowWindow(false);
			}
		
		}
	}
	else if (*p==23){


		 CProgressCtrl *progress =(CProgressCtrl *) GetDlgItem(IDC_PROGRESS6);
		 progress->SetPos(0);	
		 progress->ShowWindow(false);
	}


	return LRESULT(0);
}

void SmartSharingDlg::clearclientList()
{
	CCeButtonST *p = NULL;
	COLORREF	crBtnColor = 200;
		int i=0;
	
	for(i=IDC_BUTTON6;i<=IDC_BUTTON15;i++)
	{
	    p = (CCeButtonST *)GetDlgItem(i);
		p->ShowWindow(false);
		p->SetFont(GetFont());
		p->SetIcon(IDI_ICON6, CSize(32, 32), IDI_ICON6, CSize(32, 32));
		p->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
		p->SetAlign(CCeButtonST::ST_ALIGN_VERT);
		p->SetMenu(IDR_MENU1, m_hWnd);
	
		LPTSTR text = _T("");
		p->SetWindowTextW(text);
		p->SetParent(this);
		p->SetServer(true);
		p->SetIndex(i-IDC_BUTTON6);
		
	}

}
void 	SmartSharingDlg:: clearbuttonlist()
{
	for(int i=0;i<SIZE;i++){

		buttonList[i].cSSID = "";
		buttonList[i].cIP = "";

	}
}
void SmartSharingDlg:: clearcontrolbox()
{
	GetDlgItem(IDC_BTNSCAN)->ShowWindow(false);
	GetDlgItem(IDC_BANDWIDTH_BTN)->ShowWindow(false);
	GetDlgItem(IDC_RESTART)->ShowWindow(false);
	GetDlgItem(DC_STOP_SRV)->ShowWindow(false);
	GetDlgItem(IDC_LIST_CONT)->ShowWindow(false);
	GetDlgItem(IDC_LIST_CONN)->ShowWindow(false);
	//GetDlgItem(IDC_PROGRESS1)->ShowWindow(false);
	GetDlgItem(IDC_DEVICE1)->ShowWindow(false);
	GetDlgItem(IDC_DEVICE2)->ShowWindow(false);
	
	

}

void SmartSharingDlg::clearctlprocess()
{
	int i=0;
	CProgressCtrl *progress ;
		
	for(i=IDC_PROGRESS1;i<=IDC_PROGRESS10;i++)
	{
		progress = (CProgressCtrl *)GetDlgItem(i);
			progress->ShowWindow(false);
			progress->SetPos(0);	
	}

	
}
BOOL SmartSharingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitializeCriticalSection(&m_peerSection);

	//ModifyStyle( NULL, WS_SIZEBOX);

	COLORREF	crBtnColor = 200;

	is_send_file_flag = false;
	is_send_file_client_cancel = false;
	is_send_file_server_cancel = false;
	flag_show = 0;
	m_networkrunning = false;

	GetClientRect(&m_rect);

	MoveWindow(0, 0, 750, 630);
	CWnd *pWnd;
	CFont font;
   LOGFONT lf;
   memset(&lf,0,sizeof(LOGFONT));
   lf.lfHeight=-14;   //字体大小
   font.CreateFontIndirect(&lf);
   /*
  GetDlgItem(IDC_BUTTON1)->SetFont (&font); 
  GetDlgItem(IDC_BUTTON2)->SetFont (&font);
  GetDlgItem(IDC_BUTTON3)->SetFont (&font);
  GetDlgItem(IDC_BUTTON4)->SetFont (&font);
  GetDlgItem(IDC_FILE_CHKBOX)->SetFont (&font);
  GetDlgItem(IDC_ICS_CHKBOX)->SetFont (&font);
  GetDlgItem(IDC_DS_CHKBOX)->SetFont (&font);
  */

	pWnd = GetDlgItem(IDC_BUTTON1); 
	pWnd->MoveWindow(180,370,100,50);
	pWnd = GetDlgItem(IDC_BUTTON2);
	pWnd->MoveWindow(240,420,100,50);
	pWnd = GetDlgItem(IDC_BUTTON3);
	pWnd->MoveWindow(240,470,100,50);
	pWnd = GetDlgItem(IDC_BUTTON4);
	pWnd->MoveWindow(180,520,100,50);
	pWnd = GetDlgItem(IDC_PICTURE);
	pWnd->MoveWindow(120,420,105,89);

	pWnd = GetDlgItem(IDC_FILE_CHKBOX);
	pWnd->MoveWindow(350,460,100,25);
	pWnd = GetDlgItem(IDC_ICS_CHKBOX);
	pWnd->MoveWindow(350,480,100,25);
	pWnd = GetDlgItem(IDC_DS_CHKBOX);
	pWnd->MoveWindow(350,500,100,25);
	  
	
	int i;
	CCeButtonST *p = NULL;
	for(i=IDC_BUTTON6;i<=IDC_BUTTON10;i++){
		 p = (CCeButtonST *)GetDlgItem(i);
	
		p->MoveWindow(100+(i-IDC_BUTTON6)*60+(i-IDC_BUTTON6)*5,130,32,32);
	}
	for(i=IDC_BUTTON11;i<=IDC_BUTTON15;i++){
		 p = (CCeButtonST *)GetDlgItem(i);
	
		p->MoveWindow(100+(i-IDC_BUTTON11)*60+(i-IDC_BUTTON11)*5,180,32,32);
	}

	for(i=IDC_BUTTON16;i<=IDC_BUTTON20;i++){
		 p = (CCeButtonST *)GetDlgItem(i);
	
		p->MoveWindow(100+(i-IDC_BUTTON16)*60+(i-IDC_BUTTON16)*5,230,32,32);
	}
	for(i=IDC_BUTTON21;i<=IDC_BUTTON25;i++){
		 p = (CCeButtonST *)GetDlgItem(i);
	
		p->MoveWindow(100+(i-IDC_BUTTON21)*60+(i-IDC_BUTTON21)*5,280,32,32);
	}




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


	//m_button_show.SetFont(GetFont());
    //m_button_show.SetIcon(IDI_ICON7, CSize(32, 32), IDI_ICON7, CSize(32, 32));
    //m_button_show.SetIcon(IDI_ICON7, CSize(100, 100), IDI_ICON7, CSize(100, 100), IDI_ICON7, CSize(100, 100));
	//m_button_show.SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
	//m_button_show.SetAlign(CCeButtonST::ST_ALIGN_VERT);
	/*
	CBitmap Bitmap;
	Bitmap.LoadBitmap(IDB_BITMAP2);
HBITMAP hBitmap=(HBITMAP)Bitmap.Detach();
	m_button_show.SetBitmap(hBitmap);
	*/


	

	//if (m_button_show.GetBitmap() == NULL)
       // m_button_show.SetBitmap(::LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP2)));
	
	//if (m_button_show.GetIcon() == NULL)
       //  m_button_show.SetIcon(::LoadIcon(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ICON1)));
	

	m_button1.ShowWindow(false);
	m_button2.ShowWindow(false);
	m_button3.ShowWindow(false);
	m_button4.ShowWindow(false);
	pWnd = GetDlgItem(IDC_FILE_CHKBOX);
	pWnd->ShowWindow(false);
	pWnd = GetDlgItem(IDC_ICS_CHKBOX);
	pWnd->ShowWindow(false);
	pWnd = GetDlgItem(IDC_DS_CHKBOX);
	pWnd->ShowWindow(false);

	
	
	
	clearclientList();
	
	for(i=IDC_BUTTON16;i<=IDC_BUTTON25;i++)
	{
	    p = (CCeButtonST *)GetDlgItem(i);
		p->ShowWindow(false);
		p->SetFont(GetFont());
		p->SetIcon(IDI_ICON6, CSize(32, 32), IDI_ICON6, CSize(32, 32));
		p->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
		p->SetAlign(CCeButtonST::ST_ALIGN_VERT);
		p->SetMenu(IDR_MENU2, m_hWnd);
		LPTSTR text = _T("");
		p->SetWindowTextW(text);
		p->SetParent(this);
		p->SetServer(false);
		p->SetIndex(i-IDC_BUTTON16);
		
	}


	
	m_listCont.ModifyStyle(0, LVS_REPORT);
	m_listCont.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE);
	m_listCont.InsertColumn(0, _T("SSID"), 0, 200);
	m_listCont.InsertColumn(1, _T("Status"), 0, 60);
	m_listCont.InsertColumn(2, _T("FileSrv"), 0, 60);
	m_listCont.InsertColumn(3, _T("ICSSrv"), 0, 60);
	m_listCont.InsertColumn(4, _T("DSSrv"), 0, 60);

	m_listConn.ModifyStyle(0, LVS_REPORT);
	m_listConn.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE);
	m_listConn.InsertColumn(0, _T("SSID"), 0, 200);
	m_listConn.InsertColumn(1, _T("IP"), 0, 60);
	m_listConn.InsertColumn(2, _T("FileSrv"), 0, 60);
	m_listConn.InsertColumn(3, _T("ICSSrv"), 0, 60);
	m_listConn.InsertColumn(4, _T("DSSrv"), 0, 60);


	
	clearcontrolbox();
	clearctlprocess();

	handleWlanEventFlag = TRUE;
	CreateThread(NULL, 0, handleWlanEvent, this, 0, NULL);
	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_progress.SetRange(0, 100);
	m_LocalState.SetParent(this);
	m_LocalState.GotoNormal();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void SmartSharingDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void SmartSharingDlg::OnPaint()
{

		CPaintDC dc(this); // device context for painting


		// TODO: 在此处添加消息处理程序代码
		// 不为绘图消息调用 CDialog::OnPaint()
			CBitmap bitmap;   
	bitmap.LoadBitmap(IDB_BITMAP1);
	CBrush brush; 
	brush.CreatePatternBrush(&bitmap);  
	CBrush* pOldBrush = dc.SelectObject(&brush);  
	dc.Rectangle(0,0,750,600);



	
	


	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}


 
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR SmartSharingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void SmartSharingDlg::clearpeerList()
{
	int i;
	for(i=0;i<SIZE;i++){

		peerinformList[i].SSID ="";
		peerinformList[i].Status = "";
		peerinformList[i].FileSrv ="";
		peerinformList[i].ICSSrv ="";
		peerinformList[i].DSSrv ="";

	}
}


void SmartSharingDlg::OnBnClickedBtnscan()
{
	// TODO: 在此添加控件通知处理程序代码
		// TODO: Add your control notification handler code here

	if(CheckTransferStaus()){

		AfxMessageBox(_T("file is transfering and you can't disconnect peer"));
		return;
	}
	if (m_LocalState.GetCurrentState() == CLocalState::RECEIVINGFILE)
	{
		AfxMessageBox(_T("You are receiving files, please don't exit the window!"));
		return;
	}
	GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Scan the device nearby, please waiting ...."));
	trace("begin to scan");
	trace("Scan the device nearby, please waiting ....");
	//Disable the button so that user has not chance to click it twice before scan is finished
	//EnableWindow(false);
	hThreadScan = CreateThread(NULL, 0, ScanAP, this, 0, NULL);
	WaitForSingleObject(hThreadScan,600000);
	CString tmp = _T("");
	CString ssid;
	TCHAR ds_flag = 0;
	TCHAR ics_flag = 0;
	TCHAR file_flag = 0;
	int pos = 0;
	mfile_flag = 0;

	//clean the list first
	m_listCont.DeleteAllItems();
	clearpeerList();
	clearclientList();
	CCeButtonST *b = NULL;
	COLORREF	crBtnColor = 200;
			
	for (int i = 0; i < vecSelectedBSSEntry.size(); i++)
    {
			
		b = (CCeButtonST *)GetDlgItem(IDC_BUTTON6+i);
			if(m_Indexclient==i && !m_Indexclientflag){

					
					
					b->ShowWindow(true);
					b->SetFont(GetFont());
					b->SetIcon(IDI_ICON6, CSize(32, 32), IDI_ICON6, CSize(32, 32));
    				b->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
					b->SetAlign(CCeButtonST::ST_ALIGN_VERT);
					b->SetMenu(IDR_MENU1, m_hWnd);

			}else if(m_Indexclient==i && m_Indexclientflag){
				
	
				
					b->ShowWindow(true);
					b->SetFont(GetFont());
					b->SetIcon(IDI_ICON5, CSize(32, 32), IDI_ICON5, CSize(32, 32));
    				b->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
					b->SetAlign(CCeButtonST::ST_ALIGN_VERT);
					if(mfile_flag != '0'){
						b->SetMenu(IDR_MENU1_1, m_hWnd);
					}else{

						b->SetMenu(IDR_MENU1_2, m_hWnd);

					}
			}else{
				
					b->ShowWindow(true);
					b->SetFont(GetFont());
					b->SetIcon(IDI_ICON6, CSize(32, 32), IDI_ICON6, CSize(32, 32));
    				b->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
					b->SetAlign(CCeButtonST::ST_ALIGN_VERT);
					b->SetMenu(IDR_MENU1, m_hWnd);
				
			}
	

		USES_CONVERSION; 
		tmp.Format(_T("%20s    %x:%x:%x:%x:%x:%x"),A2T((LPCSTR)(vecSelectedBSSEntry[i].dot11Ssid.ucSSID)),
			                                       vecSelectedBSSEntry[i].dot11Bssid[0], 
												   vecSelectedBSSEntry[i].dot11Bssid[1], 
												   vecSelectedBSSEntry[i].dot11Bssid[2], 
												   vecSelectedBSSEntry[i].dot11Bssid[3], 
												   vecSelectedBSSEntry[i].dot11Bssid[4], 
												   vecSelectedBSSEntry[i].dot11Bssid[5]);
		ssid.Format(_T("%20s"),A2T((LPCSTR)(vecSelectedBSSEntry[i].dot11Ssid.ucSSID)));
		m_listCont.InsertItem(i, _T("123"));
		m_listCont.SetItemText(i, 0, ssid);
		peerinformList[i].SSID = ssid;

        if(m_LocalState.GetCurrentState() == CLocalState::CONNECTED &&
           m_LocalState.GetConnectedSSID()==ssid
          )
        {
            m_listCont.SetItemText(i, 1, _T("Connected"));
			peerinformList[i].Status = _T("Connected");
        }else{
            m_listCont.SetItemText(i, 1, _T("Not Connected"));
			peerinformList[i].Status = _T("Not Connected");

        }
		pos = ssid.ReverseFind('-');
		ds_flag = ssid.GetAt(ssid.GetLength() - 1);
		ics_flag = ssid.GetAt(ssid.GetLength() - 3);
		file_flag = ssid.GetAt(ssid.GetLength() - 5);
		mfile_flag = file_flag;
		if (file_flag == '0')
		{
			m_listCont.SetItemText(i, 2, _T("Stoped"));
			peerinformList[i].FileSrv  = _T("Stoped");
		}
		else
		{
			m_listCont.SetItemText(i, 2, _T("Running"));
			peerinformList[i].FileSrv  = _T("Running");
		}
		if (ics_flag == '0')
		{
			m_listCont.SetItemText(i, 3, _T("Stoped"));
			peerinformList[i].ICSSrv  = _T("Stoped");
		}
		else
		{
			m_listCont.SetItemText(i, 3, _T("Running"));
			peerinformList[i].ICSSrv  = _T("Running");
		}
		if (ds_flag == '0')
		{
			m_listCont.SetItemText(i, 4, _T("Stoped"));
			peerinformList[i].ICSSrv  =  _T("Stoped");
		}
		else
		{
			m_listCont.SetItemText(i, 4, _T("Running"));
			peerinformList[i].ICSSrv  =  _T("Running");
		}
	}
	GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Scan has finished."));
	trace("finish the scan.");
	//Disable the button so that user has not chance to click it twice before scan is finished
	//EnableWindow(true);
}

DWORD SmartSharingDlg::ScanAP(LPVOID lpParameter){
	
    vecSelectedBSSEntry.clear();
    int nRet = CWfdWlanSolution::Instance()->ScanBssList(vecSelectedBSSEntry, true, 1);
	
	for (int i=0; i<vecSelectedBSSEntry.size(); i++)
	{

	}

    // add text to ui control

    return 0;
}

DWORD SmartSharingDlg::HandleMessage(LPVOID lpParameter)
{
	
	//recv_sock.Receive();

    return 0;
}

BOOL SmartSharingDlg::SendFileContent2Net(char *buf, int len, UINT port, LPCTSTR dstIP, unsigned int indexSnd, unsigned int indexRes)
{
	int times = 10;
	bool bResend=true;

	while (times)
	{
		if(bResend)
		{
			rcvdataSock.SendTo(buf, len, port, dstIP);
			times--;
		}

		if (WAIT_TIMEOUT == WaitForSingleObject(hMutexFileCont, WAIT_ACK_TIME_OUT))
		{
			
			if (0 == times)
			{
				return FALSE;
			}
			else
			{
				bResend=true;
				continue;
			}
		}
		else
		{
			//package received, need to check the package no
			if(ackPkg==indexSnd){

				if(g_md5_is_true==0){

					bResend=false;				
					continue;
				}
				return TRUE;
			}else{
				bResend=false;
				continue;
			}
		}
		
	}
    return false;
}

BOOL SmartSharingDlg::SendFileContent2Net1(char *buf, int len, UINT port, LPCTSTR dstIP, unsigned int indexSnd, unsigned int indexRes)
{
	int times = 10;
	bool bResend=true;

	while (times)
	{
		if(bResend)
		{
			rcvserverdataSock.SendTo(buf, len, port, dstIP);
			times--;
		}

		if (WAIT_TIMEOUT == WaitForSingleObject(myMap[dstIP]->hMutexFileCont, WAIT_ACK_TIME_OUT))
		{
			
			if (0 == times)
			{
				return FALSE;
			}
			else
			{
				bResend=true;
				continue;
			}
		}
		else
		{
			//package received, need to check the package no
			if(indexSnd==myMap[dstIP]->ackPkg){

				if(myMap[dstIP]->md5_is_true==0){

					bResend=false;				
					continue;
				}
				return TRUE;
			}else{
				bResend=false;
				continue;
			}
		}
		
	}
    return false;
}
DWORD SmartSharingDlg::SendFile(LPVOID p)
{

	CFile sourceFile;
	CFileException ex;
	CString pszFileName;
	CString fileName;
	int pos = 0;
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("All Files (*.*)|*.*||"), NULL);
	ULONGLONG fileSize;   
	CFileStatus fileStatus;   
	LPCTSTR dstIP;
	ULONGLONG mySize = 0;
	ackPkg = 0;
	int number;

	trace("enter client begin to send file");

	/* Has disallowed when connect to ap
	if (!(((SmartSharingDlg *)p)->toClientFlag))
	{
		ShellExecute(NULL, L"open", L"netsh.exe", L"wlan set hostednetwork mode=disallow", L"", SW_HIDE);
	}
	*/

	if(((SmartSharingDlg *)p)->is_send_file_flag){

		 number = 12;
		((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)0,(LPARAM) &number);
		return  false;
	}

	((SmartSharingDlg *)p)->is_send_file_flag = true;

	dstIP = ((SmartSharingDlg *)p)->dst_ip;
	//dstIP = _T("127.0.0.1");

	

	
	if(((SmartSharingDlg *)p)->is_dragfile){

		   	 pszFileName =((SmartSharingDlg *)p)->drag_filepath;
			 
	}else{
		 if (dlgFile.DoModal())
		{
		
			 pszFileName = dlgFile.GetPathName();
	
		 }
	}
	if (pszFileName.IsEmpty())
	{
		((SmartSharingDlg *)p)->is_send_file_flag = false;
		return false;
	}
	if (CFile::GetStatus(pszFileName, fileStatus))  
	{  
		fileSize = fileStatus.m_size;  
	}
	//AfxMessageBox(pszFileName);
	pos = pszFileName.ReverseFind('\\');
	fileName = pszFileName.Right(pszFileName.GetLength() - pos -1);
	//AfxMessageBox(fileName);

	if (!sourceFile.Open(pszFileName,
      CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary , &ex))
	{
		TCHAR szError[1024];
		CString s;
		ex.GetErrorMessage(szError, 1024);
		s.Format(_T("Couldn't open source file: %1024s"), szError);
		AfxMessageBox(s);
		trace("client open file faiure\n");
		((SmartSharingDlg *)p)->is_send_file_flag = false;
		return false;
	}
	else 
	{
		UINT dwRead = 0;
		char revPkg[PKGSIZE] = {0};
		char content[CONTENTSIZE] ={0};
		char md5_s[MD5_SIZE] ={0};
		unsigned int index = 0;
		//char *pTmp = (char *)&index;
		BOOL sendFlag = FALSE;
		BOOL msgFlag=FALSE;
		DWORD waitFlag = 0;
		int fileNameLen = 0;
		// 0: cmd
		// 2~5 content length
		fileNameLen = fileName.GetLength();
		revPkg[0] = TRANS_FILE_START; // trans start
		memcpy_s(&revPkg[1], sizeof(int), (char *)&fileNameLen, sizeof(int));
		for(int i=0; i<fileName.GetLength(); i++)
		{
			revPkg[PKGCONTOFFSET+i] = fileName.GetAt(i);
		} 
		dwRead = ((SmartSharingDlg *)p)->rcvSock.SendTo(revPkg, fileNameLen+PKGCONTOFFSET, SOCK_PORT_RECV, dstIP);	
		((SmartSharingDlg *)p)->m_LocalState.GotoSendFileWaitAck();
		waitFlag = WaitForSingleObject(hMutexFileStart, 60000); /* 1min */
		
		if ((WAIT_TIMEOUT == waitFlag) || ('0' == theApp.TransFileAllowFlag)) // refused
		{
			AfxMessageBox(_T("Send file was refused by peer "));
			sourceFile.Close();
			((SmartSharingDlg *)p)->is_send_file_flag = false;
			((SmartSharingDlg *)p)->m_LocalState.GotoSendFileAckCancel();
			return false;
		}
		else 
		{
			theApp.TransFileAllowFlag = '0';
			((SmartSharingDlg *)p)->m_LocalState.GotoSendFileAckOK();
		}
		do
		{
			
			if(((SmartSharingDlg *)p)->is_send_file_client_cancel){

				trace("send file cancel");
				((SmartSharingDlg *)p)->is_send_file_client_cancel = false;
				((SmartSharingDlg *)p)->is_send_file_flag = false;
				CProgressCtrl *progress =(CProgressCtrl *) ((SmartSharingDlg *)p)->GetDlgItem(IDC_PROGRESS6);
				progress->SetPos(0);	
				progress->ShowWindow(false);
				//sourceFile.Close();

				memset(revPkg, 0, PKGSIZE);
				revPkg[0] = TRANS_FILE_CANCEL; // trans end
				memset(&revPkg[1], 0, 15);
				dwRead = ((SmartSharingDlg *)p)->rcvSock.SendTo(revPkg, PKGCONTOFFSET, SOCK_PORT_RECV, dstIP);	
				((SmartSharingDlg *)p)->m_LocalState.GotoSendFileAckEnd();
				return false;

			}
			memset(revPkg, 0, PKGSIZE);
			memset(content,0,CONTENTSIZE);
			memset(md5_s,0,MD5_SIZE);
			revPkg[0] = TRANS_FILE_CONT; // trans content

			
			dwRead = sourceFile.Read(&revPkg[PKGCONTOFFSET], CONTENTSIZE);\
			if(dwRead>=CONTENTSIZE){
				dwRead = CONTENTSIZE;
			}
			memcpy_s(&revPkg[1], sizeof(int), (char *)&dwRead, sizeof(int));
			memcpy_s(&revPkg[10], sizeof(int), (char *)&index, sizeof(int));//add the package no which is going to send
			memcpy_s(&content[0],dwRead,&revPkg[PKGCONTOFFSET],dwRead);
		//	unsigned int ret =0;
		//	ret = crc32(&content[0],dwRead);

			MD5String(md5_s,&content[0],dwRead);
			memcpy_s(&revPkg[PKGCONTOFFSET+dwRead],MD5_SIZE,md5_s,MD5_SIZE);
		//	memcpy_s(&revPkg[PKGCONTOFFSET+dwRead],sizeof(unsigned int),(char *)&ret,sizeof(unsigned int));

			mySize += dwRead;
			if (0 != mySize)
			{
				revPkg[5] = mySize*100/fileSize;
			}
			if (dwRead > 0)
			{
				number = 14;
			  ((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)0,(LPARAM) &number);

				sendFlag = ((SmartSharingDlg *)p)->SendFileContent2Net(revPkg, dwRead+PKGCONTOFFSET+MD5_SIZE, SOCK_PORT_RECV_DATA, dstIP, index, NULL);
			if (!sendFlag)
				{
					AfxMessageBox(_T("网络已断开， 传送文件停止！"));
					trace("send file failue Network error");
					((SmartSharingDlg *)p)->clearctlprocess();
					//sourceFile.Close();
					((SmartSharingDlg *)p)->is_send_file_flag = false;
						((SmartSharingDlg *)p)->is_send_file_client_cancel = false;
					 int rst;
					 rst = CWfdWlanSolution::Instance()->DisConnectToAP();
				    CProgressCtrl *progress =(CProgressCtrl *) ((SmartSharingDlg *)p)->GetDlgItem(IDC_PROGRESS6);
					progress->SetPos(0);	
					progress->ShowWindow(false);
					int number = 23;
					((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)0,(LPARAM) &number);
					  
					  number = 4;
					((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)0,(LPARAM) &number);
					((SmartSharingDlg *)p)->m_LocalState.GotoDisconnect();
					return false;
				}
				index++;
			
	
			
				message msg;
				msg.index = 0;
				msg.percent = revPkg[5];
				int number = 17;
			   ((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)&msg,(LPARAM) &number);
			}else{
			
			}
			
		} while (dwRead > 0);

		memset(revPkg, 0, PKGSIZE);
		revPkg[0] = TRANS_FILE_END; // trans end
		memset(&revPkg[1], 0, 15);
		dwRead = ((SmartSharingDlg *)p)->rcvSock.SendTo(revPkg, PKGCONTOFFSET, SOCK_PORT_RECV, dstIP);	
		((SmartSharingDlg *)p)->m_LocalState.GotoSendFileAckEnd();

		WaitForSingleObject(hMutexFileEnd, INFINITE);
	
		 number = 13;
	    ((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)0,(LPARAM) &number);

		CProgressCtrl *progress =(CProgressCtrl *) ((SmartSharingDlg *)p)->GetDlgItem(IDC_PROGRESS6);
		progress->SetPos(0);	
		progress->ShowWindow(false);
		
		/*
		message msg;
		msg.index = 0;
		msg.percent = 0;
		number = 18;
	    ((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)&msg,(LPARAM) &number);
		*/
	
	}

	((SmartSharingDlg *)p)->is_send_file_flag = false;
	trace("quit client send file");
	
	return true;
}

DWORD SmartSharingDlg::ServerSendFile(LPVOID p)
{
	CFile sourceFile;
	CFileException ex;
	CString pszFileName;
	CString fileName;
	int pos = 0;
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("All Files (*.*)|*.*||"), NULL);
	ULONGLONG fileSize;   
	CFileStatus fileStatus;   
	LPCTSTR dstIP;
	ULONGLONG mySize = 0;
	ackPkg = 0;
	int number;
	int b_index =((SmartSharingDlg *)p)-> m_Index;

	trace("enter the server send file");

	if(b_index >=5){

		AfxMessageBox(_T("the number of sending files  is over 5"));
		return false;
	}

	dstIP = ((SmartSharingDlg *)p)->dst_ip;


	if(myMap[dstIP]->send_flag){

		 number = 12;
		((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)0,(LPARAM) &number);
		return  false;
	}


	myMap[dstIP]->send_flag = true;

	
	if(((SmartSharingDlg *)p)->is_dragfile){

		   	 pszFileName =((SmartSharingDlg *)p)->drag_filepath;
			 
	}else{
		 if (dlgFile.DoModal())
		{
		
			 pszFileName = dlgFile.GetPathName();
	
		 }
	}
	if (pszFileName.IsEmpty())
	{
		myMap[dstIP]->send_flag = false;
		return false;
	}
	if (CFile::GetStatus(pszFileName, fileStatus))  
	{  
		fileSize = fileStatus.m_size;  
	}
	//AfxMessageBox(pszFileName);
	pos = pszFileName.ReverseFind('\\');
	fileName = pszFileName.Right(pszFileName.GetLength() - pos -1);
	//AfxMessageBox(fileName);

	if (!sourceFile.Open(pszFileName,
      CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary , &ex))
	{
		TCHAR szError[1024];
		CString s;
		ex.GetErrorMessage(szError, 1024);
		s.Format(_T("Couldn't open source file: %1024s"), szError);
		trace("server open the file failure");
		AfxMessageBox(s);
		myMap[dstIP]->send_flag = false;
		return false;
	}
	else 
	{
		UINT dwRead = 0;
		char revPkg[PKGSIZE] = {0};
		char content[CONTENTSIZE] ={0};
		char md5_s[MD5_SIZE] ={0};
		unsigned int index = 0;
		//char *pTmp = (char *)&index;
		BOOL sendFlag = FALSE;
		BOOL msgFlag=FALSE;
		int fileNameLen = 0;
		DWORD waitFlag = 0;
		// 0: cmd
		// 2~5 content length
		fileNameLen = fileName.GetLength();
		revPkg[0] = TRANS_FILE_START; // trans start
		memcpy_s(&revPkg[1], sizeof(int), (char *)&fileNameLen, sizeof(int));
		for(int i=0; i<fileName.GetLength(); i++)
		{
			revPkg[PKGCONTOFFSET+i] = fileName.GetAt(i);
		} 
		dwRead = ((SmartSharingDlg *)p)->rcvServerSock.SendTo(revPkg, fileNameLen+PKGCONTOFFSET, SOCK_PORT_RECV_SERVER, dstIP);	
		waitFlag = WaitForSingleObject(myMap[dstIP]->hMutexFileStart, 60000); /* 15s */

		if ((WAIT_TIMEOUT == waitFlag) || ('0' == theApp.TransFileAllowFlag)) // refused
		{
			AfxMessageBox(_T("Send file was refused by peer "));
			sourceFile.Close();
			myMap[dstIP]->send_flag = false;
			((SmartSharingDlg *)p)->m_LocalState.GotoSendFileAckCancel();
			return false;
		}
		else 
		{
			theApp.TransFileAllowFlag = '0';
			((SmartSharingDlg *)p)->m_LocalState.GotoSendFileAckOK();
		}
		//to do, should add the resend action in case the package lost

		do
		{
				if(((SmartSharingDlg *)p)->is_send_file_server_cancel){

				trace("send file cancel");
				((SmartSharingDlg *)p)->is_send_file_server_cancel = false;
				//sourceFile.Close();
				//fclose(myMap[dstIP]->f);
				myMap[dstIP]->send_flag = false;
				CProgressCtrl *progress =(CProgressCtrl *) ((SmartSharingDlg *)p)->GetDlgItem(IDC_PROGRESS6+b_index);
				progress->SetPos(0);	
				progress->ShowWindow(false);

				memset(revPkg, 0, PKGSIZE);
				revPkg[0] = TRANS_FILE_CANCEL; // trans end
				memcpy_s(&revPkg[1], sizeof(int), (char *)&b_index, sizeof(int));
				dwRead = ((SmartSharingDlg *)p)->rcvSock.SendTo(revPkg, PKGCONTOFFSET, SOCK_PORT_RECV, dstIP);	
				((SmartSharingDlg *)p)->m_LocalState.GotoSendFileAckEnd();
				return false;

			}

			memset(revPkg, 0, PKGSIZE);
			memset(content,0,CONTENTSIZE);
			memset(md5_s,0,MD5_SIZE);
			revPkg[0] = TRANS_FILE_CONT; // trans content

			
			dwRead = sourceFile.Read(&revPkg[PKGCONTOFFSET], CONTENTSIZE);\
			if(dwRead>=CONTENTSIZE){
				dwRead = CONTENTSIZE;
			}
			memcpy_s(&revPkg[1], sizeof(int), (char *)&dwRead, sizeof(int));
			memcpy_s(&revPkg[10], sizeof(int), (char *)&index, sizeof(int));//add the package no which is going to send
			memcpy_s(&content[0],dwRead,&revPkg[PKGCONTOFFSET],dwRead);
		//	unsigned int ret =0;
		//	ret = crc32(&content[0],dwRead);

			MD5String(md5_s,&content[0],dwRead);
			memcpy_s(&revPkg[PKGCONTOFFSET+dwRead],MD5_SIZE,md5_s,MD5_SIZE);
		//	memcpy_s(&revPkg[PKGCONTOFFSET+dwRead],sizeof(unsigned int),(char *)&ret,sizeof(unsigned int));

			mySize += dwRead;
			if (0 != mySize)
			{
				revPkg[5] = mySize*100/fileSize;
			}
			if (dwRead > 0)
			{
				number = 14;
			  ((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)0,(LPARAM) &number);

				sendFlag = ((SmartSharingDlg *)p)->SendFileContent2Net1(revPkg, dwRead+PKGCONTOFFSET+MD5_SIZE, SOCK_PORT_RECV_SERVER_DATA, dstIP, index, NULL);
			//	sendFlag = ((SmartSharingDlg *)p)->SendFileContent2Net(revPkg, dwRead+PKGCONTOFFSET+sizeof(unsigned int), SOCK_PORT_RECV, dstIP, index, NULL);
				if (!sendFlag)
				{
					AfxMessageBox(_T("网络已断开， 传送文件停止！"));
					((SmartSharingDlg *)p)->clearctlprocess();
				//	sourceFile.Close();
					myMap[dstIP]->send_flag = false;
				   ((SmartSharingDlg *)p)->is_send_file_server_cancel  = false;
				//	fclose(myMap[dstIP]->f);
					int rst;
					rst = CWfdWlanSolution::Instance()->DisConnectToAP();
				    number = 22;
					((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)0,(LPARAM) &number);
					CProgressCtrl *progress =(CProgressCtrl *) ((SmartSharingDlg *)p)->GetDlgItem(IDC_PROGRESS6+b_index);
					progress->SetPos(0);	
					progress->ShowWindow(false);
					((SmartSharingDlg *)p)->m_LocalState.GotoDisconnect();

					return false;
				}
				index++;
			
	
			//	((SmartSharingDlg *)p)->m_progress.SetPos(revPkg[5]);
				message msg;
				msg.index = b_index;
				msg.percent = revPkg[5];
				int number = 17;
			   ((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)&msg,(LPARAM) &number);
			}else{
				//to do, what if readfile returns 0 bytes
			}
			
		} while (dwRead > 0);

		memset(revPkg, 0, PKGSIZE);
		revPkg[0] = TRANS_FILE_END; // trans end
		memset(&revPkg[1], 0, 15);
		dwRead = ((SmartSharingDlg *)p)->rcvServerSock.SendTo(revPkg, PKGCONTOFFSET, SOCK_PORT_RECV_SERVER, dstIP);	
		WaitForSingleObject(myMap[dstIP]->hMutexFileEnd, 10000); /* 10s */
		((SmartSharingDlg *)p)->m_LocalState.GotoSendFileAckEnd();
		 number = 13;
	    ((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)0,(LPARAM) &number);
		
		//((SmartSharingDlg *)p)->m_progress.SetPos(0);
		/*
		message msg;
		msg.index = b_index;
		msg.percent = 0;
		number = 18;
	     ((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)&msg,(LPARAM) &number);
		 */

		CProgressCtrl *progress =(CProgressCtrl *) ((SmartSharingDlg *)p)->GetDlgItem(IDC_PROGRESS6+b_index);
		progress->SetPos(0);	
		progress->ShowWindow(false);
	}

	myMap[dstIP]->send_flag = false;
	trace("quit the server send file");
	
	return true;
}
void SmartSharingDlg::addClient(CString mac)
{
	int ListCount=0;
	LPTSTR lpszText = new TCHAR[100];
	CString SSIDInList;
	int index =0;
	int j = 0;
	CString cSSID;
	CString cIP;
	trace("enter begin to addclient");

	EnterCriticalSection(&m_clientChangeCond);
//	ListCount = m_listConn.GetItemCount();
//	ListCount = SIZE;
	for (index=0; index<CLIENT_NUM; index++)
	{
		if(clients[index].mac == mac)
		{
			break;
		}
	}
	if (index >= CLIENT_NUM)
	{
		LeaveCriticalSection(&m_clientChangeCond);
		return;
	}
	cSSID = clients[index].ssid;
	cIP = clients[index].ip;
	LeaveCriticalSection(&m_clientChangeCond);
	int i;
	
	for(i=0;i<SIZE;i++)
	{
		if(buttonList[i].cSSID ==cSSID)
		{
			buttonList[i].cIP = cIP;
			break;

		}
	}
	if(i==SIZE){
		
		for(i=0;i<SIZE;i++)
		{
			if(buttonList[i].cSSID==""){
				buttonList[i].cSSID = cSSID;
				buttonList[i].cIP = cIP;
				break;
			}
		}
	}
	

	myMap[cIP]->hMutexFileStart = CreateEvent(NULL, FALSE, FALSE, NULL);
	myMap[cIP]->hMutexFileEnd= CreateEvent(NULL, FALSE, FALSE, NULL);
	myMap[cIP]->hMutexFileCont= CreateEvent(NULL, FALSE, FALSE, NULL);
	myMap[cIP]->send_flag = false;
	
	int status = 0;
	int number = 3;
	SendMessage(WM_TEST, (WPARAM)&status,(LPARAM) &number);
  
	/*
	for(j=0;j<ListCount;j++)
	{
		m_listConn.GetItemText(j,0,lpszText,100);
		SSIDInList=lpszText;
		if(SSIDInList == cSSID)
		{
			m_listConn.SetItemText(j, 1, cIP);
			break;
		}
	}
	if(j>=ListCount)
	{
		m_listConn.InsertItem(0, _T(""));
		m_listConn.SetItemText(0, 0, cSSID);
		m_listConn.SetItemText(0, 1, cIP);
	}*/
	trace("leave the    addclient");
}



void SmartSharingDlg::delClient(CString mac)
{
	int index = 0;
	int ListCount = 0;
	LPTSTR lpszText = new TCHAR[100];
	CString SSIDInList;
	CString cSSID;
	trace("enter the delClient");
	/*
	ListCount = m_listConn.GetItemCount();
	if (0 >= ListCount)
	{
		return;
	}
	*/
	EnterCriticalSection(&m_clientChangeCond);
	for (index=0; index<CLIENT_NUM; index++)
	{
		if(clients[index].mac == mac)
		{
			break;
		}
	}	
	if (index >= CLIENT_NUM)
	{
		LeaveCriticalSection(&m_clientChangeCond);
		return;
	}
	cSSID = clients[index].ssid;
	clients[index].port = 0;
	LeaveCriticalSection(&m_clientChangeCond);
	
	int i;
	
	for(i=0;i<SIZE;i++)
	{
		if(buttonList[i].cSSID==cSSID){
			buttonList[i].cSSID ="";
			buttonList[i].cIP = "";
		
		}
	}
	int status = 1;
	int number = 3;
	SendMessage(WM_TEST, (WPARAM)&status,(LPARAM) &number);
	/*
	for(int j=0;j<ListCount;j++)
	{
		m_listConn.GetItemText(j,0,lpszText,100);
		SSIDInList=lpszText;
		if(SSIDInList == cSSID)
		{
			m_listConn.DeleteItem(j);
		}
	}
	*/
	is_send_file_flag = false;
	if(dst_ip!="")
		myMap[dst_ip]->send_flag = false;
	trace("leave the delClient");
	
}
DWORD SmartSharingDlg::handleCheckNetworkLiveEvent(LPVOID p)
{


	while(((SmartSharingDlg *)p)->m_networkrunning){

					  DWORD dw;      
				 	bool bConnect=InternetCheckConnection(L"http://www.baidu.com",FLAG_ICC_FORCE_CONNECTION ,0);
					if(bConnect==false){

										CProgressCtrl *progress;
										int i;
						for(i=0;i<5;i++){
						
								COLORREF	crBtnColor = 200;
								CCeButtonST *b =NULL;
								b =(CCeButtonST *)((SmartSharingDlg *)p)->GetDlgItem(IDC_BUTTON16+i);
								
								b->ShowWindow(false);
							

								progress =(CProgressCtrl *)((SmartSharingDlg *)p)->GetDlgItem(IDC_PROGRESS1+i);
								progress->SetPos(0);
							
								progress->ShowWindow(false);

								progress =(CProgressCtrl *)((SmartSharingDlg *)p)->GetDlgItem(IDC_PROGRESS6+i);
								progress->SetPos(0);
								progress->ShowWindow(false);
								((SmartSharingDlg *)p)->GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("file transfering is  interrupted."));
								((SmartSharingDlg *)p)->m_LocalState.GotoReceiveFileCancel();
								Sleep(1000);
								((SmartSharingDlg *)p)->GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Disconnected"));
						
					}

						return 0;
							
					}else{

						//printf("aaaaaaaaaaaaaaaa\n");
					}

				
				
				
				   Sleep(5000);

	}


	return 0;
}


DWORD SmartSharingDlg::handleWlanEvent(LPVOID p)
{
	WLANEVTTYPE type = WLAN_EVT_INVAILD;
	CString mac; 


	while (((SmartSharingDlg *)p)->handleWlanEventFlag)
	{
		WaitForSingleObject(hPeerChangeEvent, INFINITE);
		EnterCriticalSection(&m_PeerChangeCond);
		type = wlanEvtType;
		mac = rsnMac;
		LeaveCriticalSection(&m_PeerChangeCond);
		switch (type)
		{
		case WLAN_EVT_AP_PEER_DEPARTED:
			{
				((SmartSharingDlg *)p)->delClient(mac);
				
			}
			break;
		case WLAN_EVT_AP_PEER_ARRIVED:
			{
				((SmartSharingDlg *)p)->addClient(mac);
				
			}
			break;
		case WLAN_EVT_STA_DISCONNECT:
			{
				int status = 0;
				int number =4;
				CString dstIP ;
				((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)&status,(LPARAM) &number);	
				((SmartSharingDlg *)p)->m_LocalState.GotoDisconnect();
				((SmartSharingDlg *)p)->is_send_file_flag = false;
				dstIP	= ((SmartSharingDlg *)p)->dst_ip;
				if(dstIP!="" ){
				     	 myMap[dstIP]->send_flag = false;
				}

			}
			break;
		default:
			{
				break;
			}
		}
	}
	return 0;
}


void SmartSharingDlg::OnBnClickedBandwidthBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if(CheckTransferStaus()){
		AfxMessageBox(_T("file is transfering and you can't disconnect peer"));
		return;
	}
	if (m_LocalState.GetCurrentState() == CLocalState::RECEIVINGFILE)
	{
		AfxMessageBox(_T("You are receiving files, please don't exit the window!"));
		return;
	}
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

	trace("quit the  bandwidthbutton click");
}

BOOL SmartSharingDlg::PreTranslateMessage(MSG* pMsg)
{
	POSITION pos = NULL;
	int index = 0;
	CString ssid;

	if(WM_RBUTTONDOWN==pMsg->message)
	{
			if(pMsg->hwnd==m_listCont.m_hWnd)
			{
				pos = m_listCont.GetFirstSelectedItemPosition();
				if(pos==NULL)
				{
					return CDialog::PreTranslateMessage(pMsg);
				}
				index = (int)m_listCont.GetNextSelectedItem(pos);
				if (-1 == index)
				{
					return CDialog::PreTranslateMessage(pMsg);
				}
				DWORD dwPos=GetMessagePos();
				CPoint point(LOWORD(dwPos),HIWORD(dwPos));
				CMenu menu;
				VERIFY(menu.LoadMenu(IDR_MENU1));
				CMenu *popup=menu.GetSubMenu(0);
				USES_CONVERSION; 
				ssid.Format(_T("%20s"),A2T((LPCSTR)(vecSelectedBSSEntry[index].dot11Ssid.ucSSID)));
				if(m_LocalState.GetCurrentState() == CLocalState::CONNECTED &&
						m_LocalState.GetConnectedSSID()==ssid)
				{
					menu.EnableMenuItem(ID_MENU_CONNECT, MF_DISABLED|MF_GRAYED);
					menu.EnableMenuItem(ID_MENU_DISCONNECT,MF_ENABLED); 
					menu.EnableMenuItem(ID_MENU_SENDFILE,MF_ENABLED); 
				}
				else
				{
					menu.EnableMenuItem(ID_MENU_CONNECT, MF_ENABLED);
					menu.EnableMenuItem(ID_MENU_DISCONNECT,MF_DISABLED|MF_GRAYED); 
					menu.EnableMenuItem(ID_MENU_SENDFILE,MF_DISABLED|MF_GRAYED); 
				}
				ASSERT(popup!=NULL);
				popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
			}
			if(pMsg->hwnd==m_listConn.m_hWnd)
			{
				pos = m_listConn.GetFirstSelectedItemPosition();
				if(pos==NULL)
				{
					return CDialog::PreTranslateMessage(pMsg);
				}

				DWORD dwPos=GetMessagePos();
				CPoint point(LOWORD(dwPos),HIWORD(dwPos));
				CMenu menu;
				VERIFY(menu.LoadMenu(IDR_MENU2));
				CMenu *popup=menu.GetSubMenu(0);
				ASSERT(popup!=NULL);
				popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
			}
	}
		return CDialog::PreTranslateMessage(pMsg);

}

void SmartSharingDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnRButtonDown(nFlags, point);
}

BOOL SmartSharingDlg::isConnected()
{
    if(m_LocalState.GetCurrentState() == CLocalState::CONNECTED)
    {
        return false;
    }else{
        return false;
    }
}

CString SmartSharingDlg::GetConnectedSSID()
{
	return m_LocalState.GetConnectedSSID();
}

BOOL SmartSharingDlg::hasService(int index)
{
	CString ssid;
	TCHAR ds_flag = 0;
	TCHAR ics_flag = 0;
	TCHAR file_flag = 0;

	USES_CONVERSION; 
	ssid.Format(_T("%20s"),A2T((LPCSTR)(vecSelectedBSSEntry[index].dot11Ssid.ucSSID)));
	ds_flag = ssid.GetAt(ssid.GetLength() - 1);
	ics_flag = ssid.GetAt(ssid.GetLength() - 3);
	file_flag = ssid.GetAt(ssid.GetLength() - 5);
	if (('0' == ds_flag) && ('0' == ics_flag) && ('0' == file_flag))
	{
		return FALSE;
	}
	return TRUE;
}

DWORD SmartSharingDlg::connect2Ap(_In_  LPVOID p)
{
    CString strSecondaryKey(_T("11111111"));
    in_addr ip_local_address;
    in_addr ip_gateway_address;
	int nRet = -1;
	// Get the indexes of all the selected items.
	CString content;
	int index = 0;
	CString s;
	CString ip_str;
	BYTE revPkg[PKGSIZE] = {0};
	DWORD sendFlag = 0;
	int len = 0;
	char *tmp = NULL;
	CString SSIDConnected;
	int times = 0;


	trace("begin to connect the server");

	int number;


	 EnterCriticalSection(&(((SmartSharingDlg *)p)->m_peerSection));
		index= ((SmartSharingDlg *)p)->m_Index;
	 LeaveCriticalSection(&(((SmartSharingDlg *)p)->m_peerSection));

	// number = 111;
	//((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)index,(LPARAM) &number);
	
	/*
	POSITION pos = ((SmartSharingDlg *)p)->m_listCont.GetFirstSelectedItemPosition();
	if(pos==NULL)
	{
		return 0;
	}
	index = (int)(((SmartSharingDlg *)p)->m_listCont.GetNextSelectedItem(pos));
	if (-1 == index)
	{
		return 0;
	}
	*/
	if (!(((SmartSharingDlg *)p)->hasService(index)))
	{
		AfxMessageBox(_T("There's no available service, so connection is not allowed."));
		trace("There's no available service, so connection is not allowed.");
		return 0;
	}
	if ((((SmartSharingDlg *)p)->isConnected()))
	{
		CString ssid;
		USES_CONVERSION; 
		ssid.Format(_T("%20s"),A2T((LPCSTR)(vecSelectedBSSEntry[index].dot11Ssid.ucSSID)));
		
        if(((SmartSharingDlg *)p)->GetConnectedSSID()==ssid)
		{
			AfxMessageBox(_T("You've connected to this one, no need to re-connect."),MB_OK);
			trace("You've connected to this one, no need to re-connect.");
			return 0;
		} 

		int rst = AfxMessageBox(_T("You've connected to " +ssid + ", do you want to disconnect from it and connect to the new one?"),
					MB_YESNO);
		if (IDNO == rst)
		{
			return 0;
		}
		else
		{
			//user has agrees to disconnect
			/*
			for(int i=0; i<(((SmartSharingDlg *)p)->m_listCont.GetItemCount()); i++)
			{
				(((SmartSharingDlg *)p)->m_listCont).SetItemText(i, 1, _T("Not Connected"));
			}*/
			for(int i=0;i<vecSelectedBSSEntry.size();i++)
			{
				((SmartSharingDlg *)p)->peerinformList[i].Status = _T("Not Connected");
			}

        }
	}
	((SmartSharingDlg *)p)->GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Connecting.... please wait a moment "));

	 	number = 111;
		((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)0,(LPARAM) &number);
	//((SmartSharingDlg *)p)->setconnectImage(index);

	//in the lib, connectToAP will call disconnect regardless what's the current state, so need to check if it is connecting
	//to the ap which has connected before this point
	nRet = CWfdWlanSolution::Instance()->ConnectToAP(index, strSecondaryKey, ip_local_address, ip_gateway_address);

	if (ip_local_address.S_un.S_un_b.s_b1 == 169)
	{
		//((SmartSharingDlg *)p)->GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("It takes long time to get IP, please try again later. "));
		//return 0;
		CString m_wlanIP;
		CString m_SoftApIP;
		CString path;
		::GetCurrentDirectory(2000,path.GetBuffer(2000));
		path.ReleaseBuffer();
		path +="\\launch.ini";
		path.Replace('\\', '/');
		::GetPrivateProfileStringW(_T("Setting"),_T("wlanIP"),_T("192.168.137.153"),m_wlanIP.GetBuffer(MAX_PATH),MAX_PATH, path);  
		::GetPrivateProfileStringW(_T("Setting"),_T("SoftApIP"),_T("192.168.137.1"),m_SoftApIP.GetBuffer(MAX_PATH),MAX_PATH, path);
		m_wlanIP.ReleaseBuffer();
		m_SoftApIP.ReleaseBuffer();
		theApp.setSoftApAddr(m_wlanIP, _T("NULL"));
	}
	ip_str.Format(_T("%d.%d.%d.%d"), ip_local_address.S_un.S_un_b.s_b1, ip_local_address.S_un.S_un_b.s_b2,
			ip_local_address.S_un.S_un_b.s_b3, ip_local_address.S_un.S_un_b.s_b4);
	//use the local IP to caculate the gateway IP, sometimes there are more than one gateway IPs and the 1st one is like 192.168.137.2
	((SmartSharingDlg *)p)->ip_gateway.Format(_T("%d.%d.%d.1"), ip_local_address.S_un.S_un_b.s_b1, ip_local_address.S_un.S_un_b.s_b2,
			ip_local_address.S_un.S_un_b.s_b3);
	if ((nRet == WFD_ERROR_SUCCESS) || (nRet == WFD_ERROR_WLAN_GET_IP_FAILED))
	{
		s.Format(_T("Connected, host ip: %s, gateway ip: %s"), ip_str, ((SmartSharingDlg *)p)->ip_gateway);
		 AfxMessageBox (s);	
		/*
		if(!sndSock.Create(SOCK_PORT_SEND, SOCK_DGRAM, FD_READ | FD_WRITE, ip_str))
		{
			AfxMessageBox(_T("create socket failed"));
		}
		*/
		//Have to remove the virtual adpater, otherwise the package will send the sender itself
		ShellExecute(NULL, L"open", L"netsh.exe", L"wlan set hostednetwork mode=disallow", L"", SW_HIDE);
	    Sleep(3000);

		memset(revPkg, 0, PKGSIZE);
		revPkg[0] = CONNECT_TO_SERVER; // trans end
		len = strSSID.GetLength();
		memcpy_s(&revPkg[1], sizeof(int), (char *)&len, sizeof(int));	
		for(int i=0; i<strSSID.GetLength(); i++)
		{
			revPkg[PKGCONTOFFSET+i] = strSSID.GetAt(i);
		}
		tmp = (LPSTR)(LPCTSTR)strSSID;
		for (int i=0, j=0; i<strSSID.GetLength(); i++, j+=2)
		{
			revPkg[PKGCONTOFFSET+i] = tmp[j];
		}
		for (int i=0; i<6; i++)
		{
			revPkg[PKGCONTOFFSET+32+i] = macAddr[i];
		}
		//memcpy_s(&revPkg[PKGCONTOFFSET], strSSID.GetLength(), tmp, strSSID.GetLength());
		((SmartSharingDlg *)p)->rcvSock.SendTo(revPkg, PKGCONTOFFSET+32+6, SOCK_PORT_RECV, ((SmartSharingDlg *)p)->ip_gateway);	
		((SmartSharingDlg *)p)->rcvServerSock.SendTo(revPkg, PKGCONTOFFSET+32+6, SOCK_PORT_RECV_SERVER, ((SmartSharingDlg *)p)->ip_gateway);
		((SmartSharingDlg *)p)->rcvdataSock.SendTo(revPkg, PKGCONTOFFSET+32+6, SOCK_PORT_RECV_DATA, ((SmartSharingDlg *)p)->ip_gateway);
		((SmartSharingDlg *)p)->rcvserverdataSock.SendTo(revPkg, PKGCONTOFFSET+32+6, SOCK_PORT_RECV_SERVER_DATA, ((SmartSharingDlg *)p)->ip_gateway);
		
		//to do: block here forever?
		//to do, should add the resend action in case the package lost
		/*
		sendFlag = ((SmartSharingDlg *)p)->sendMsg2Net(revPkg, fileNameLen+PKGCONTOFFSET, SOCK_PORT_RECV, dstIP, NULL, NULL);
		if (!sendFlag)
		{
			AfxMessageBox(_T("start file Network error"));
			sourceFile.Close();
			return false;
		}
		*/
		sendFlag = WaitForSingleObject(hMutexConnectToServer, 15000);
		
		if(WAIT_OBJECT_0 != sendFlag){

			if(WAIT_TIMEOUT ==sendFlag){
				((SmartSharingDlg *)p)->GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("No connection. Connection request has been refused."));
				AfxMessageBox(_T(" Connection request has been refused."));
			}else if(WAIT_FAILED ==sendFlag){

				AfxMessageBox(_T(" sentence return   error"));
			}

				number = 2;
			    ((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)&index,(LPARAM) &number);
				Sleep(3000);
				return false;
		}
		
		if ('0' == theApp.connAllowFlag) // the server not allowed
		{
			((SmartSharingDlg *)p)->GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Connection request has been refused. "));
			CWfdWlanSolution::Instance()->DisConnectToAP();
			
			ShellExecute(NULL, L"open", L"netsh.exe", L"wlan set hostednetwork mode=allow", L"", SW_HIDE);
			Sleep(3000);
			theApp.StartSoftAP(theApp.fileFlag, theApp.icsFlag, theApp.dsFlag);
		   
			number = 1;
		   ((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)&index,(LPARAM) &number);
			
			return 0;
		}



		((SmartSharingDlg *)p)->GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Connected."));
		(((SmartSharingDlg *)p)->m_listCont).SetItemText(index, 1, _T("Connected"));

		CCeButtonST *b = NULL;
		COLORREF	crBtnColor = 200;
		
		
		
		

		USES_CONVERSION; 
		SSIDConnected.Format(_T("%20s"),A2T((LPCSTR)(vecSelectedBSSEntry[index].dot11Ssid.ucSSID)));
		(((SmartSharingDlg *)p)->m_LocalState).GoToConnected(SSIDConnected);
		
		number = 2;
		((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)&index,(LPARAM) &number);
		//((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)index,(LPARAM) &number);

		

	}
	else
	{
		AfxMessageBox(_T("Connected failed"));
		trace("connect the server  failed");
		ShellExecute(NULL, L"open", L"netsh.exe", L"wlan set hostednetwork mode=allow", L"", SW_HIDE);
		Sleep(3000);
		theApp.StartSoftAP(theApp.fileFlag, theApp.icsFlag, theApp.dsFlag);
		number = 1;
		((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)&index,(LPARAM) &number);
		
	}
	return 0;
}

void SmartSharingDlg::setconnectImage(int index){


			COLORREF	crBtnColor = 200;
			CCeButtonST *b =NULL;
			b =(CCeButtonST *)GetDlgItem(IDC_BUTTON6+index);
			b->ShowWindow(true);
			b->SetFont(GetFont());
			b->SetIcon(IDI_ICON6, CSize(32, 32), IDI_ICON6, CSize(32, 32));
    		b->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
			b->SetAlign(CCeButtonST::ST_ALIGN_VERT);
			b->SetMenu(IDR_MENU9, m_hWnd);

}

void SmartSharingDlg::OnMenuConnect()
{
	// TODO: 在此添加命令处理程序代码
	HANDLE h_sendFile = NULL;
	h_sendFile = CreateThread(NULL, 0, connect2Ap, this, 0, NULL);
}
int SmartSharingDlg::CheckTransferStaus(){

/*
		CProgressCtrl *progress;
	int i;
	for( i=0;i<=9;i++){
			progress =(CProgressCtrl *)GetDlgItem(IDC_PROGRESS1+i);
			if(progress->IsWindowVisible()){
					
				return 1;
			}
	}
	return 0;
	*/
		if ((m_LocalState.GetCurrentState() == CLocalState::SENDFILEACKOK)
		|| (m_LocalState.GetCurrentState() == CLocalState::SENDFILEWAITACK))
		{
			return 1;
		}
		return 0;
}

void SmartSharingDlg::OnMenuDisconnect()
{
	// TODO: 在此添加命令处理程序代码
	int rst = -1;
	int index = -1;
	/*
	POSITION pos=m_listCont.GetFirstSelectedItemPosition();
	if(pos==NULL)
	{
		return;
	}
	index = (int)m_listCont.GetNextSelectedItem(pos);
	if (-1 == index)
	{
		return;
	}
	*/

	if(CheckTransferStaus()){
		AfxMessageBox(_T("file is transfering and you can't disconnect peer"));
		return;
	}
	if ((m_LocalState.GetCurrentState() == CLocalState::SENDFILEWAITACK)
	|| (m_LocalState.GetCurrentState() == CLocalState::SENDFILEACKOK))
	{
			AfxMessageBox(_T("Please wait until the send file operation finished."));
			return;
	}
	if ((m_LocalState.GetCurrentState() == CLocalState::RECEIVINGFILE))
	{
		AfxMessageBox(_T("Please wait until the receiving file operation finished."));
		return;
	}
	m_LocalState.GotoDisconnect();

	rst = CWfdWlanSolution::Instance()->DisConnectToAP();

     EnterCriticalSection(&m_peerSection);
		index= m_Index;
	 LeaveCriticalSection(&m_peerSection);

	 int number = 2;
	SendMessage(WM_TEST, (WPARAM)&index,(LPARAM) &number);
	number = 8;
	SendMessage(WM_TEST, (WPARAM)&index,(LPARAM) &number);
	  
	/*
	CCeButtonST *p= NULL;
    COLORREF	crBtnColor = 200;
	p = (CCeButtonST *)GetDlgItem(IDC_BUTTON6+index);
	p->ShowWindow(true);
	p->SetFont(GetFont());
	p->SetIcon(IDI_ICON6, CSize(32, 32), IDI_ICON6, CSize(32, 32));
	p->SetColor(CCeButtonST::BTNST_COLOR_BK_IN, crBtnColor);
	p->SetAlign(CCeButtonST::ST_ALIGN_VERT);
	p->SetMenu(IDR_MENU1, m_hWnd);
	*/
	/*
	if (WFD_ERROR_SUCCESS == rst)
	{
		m_listCont.SetItemText(index, 1, _T("DisConnected"));
		connectedItem = -1;
		m_LocalState.GotoDisconnect();
	}
	*/

}


void SmartSharingDlg::DisConnect(int index)
{
	// TODO: 在此添加命令处理程序代码
	int rst = -1;

	m_LocalState.GotoDisconnect();

	rst = CWfdWlanSolution::Instance()->DisConnectToAP();

	 int number = 2;
	SendMessage(WM_TEST, (WPARAM)&index,(LPARAM) &number);
}

void SmartSharingDlg::OnMenuSendfile()
{
	// TODO: 在此添加命令处理程序代码
	if ((m_LocalState.GetCurrentState() == CLocalState::SENDFILEWAITACK)
		|| (m_LocalState.GetCurrentState() == CLocalState::SENDFILEACKOK))
	{
		AfxMessageBox(_T("Please wait until the last send file operation finished."));
		return;
	}
	if ((m_LocalState.GetCurrentState() == CLocalState::RECEIVINGFILE))
	{
		AfxMessageBox(_T("Please wait until the receiving file operation finished."));
		return;
	}
	HANDLE h_sendFile = NULL;
	toClientFlag = FALSE;
	dst_ip = ip_gateway;
	is_dragfile = false;
	h_sendFile = CreateThread(NULL, 0, SendFile, this, 0, NULL);
}


void SmartSharingDlg::OnLbnSelchangeListPeer()
{
	// TODO: 在此添加控件通知处理程序代码
}



void SmartSharingDlg::OnBnClickedFileChkbox()
{
	// TODO: 在此添加控件通知处理程序代码
	theApp.fileFlag = m_fileSrvFlag.GetCheck();
}


void SmartSharingDlg::OnBnClickedIcsChkbox()
{
	// TODO: 在此添加控件通知处理程序代码
	theApp.icsFlag = m_ICSSrvFlag.GetCheck();
}   



void SmartSharingDlg::OnBnClickedDsChkbox()
{
	// TODO: 在此添加控件通知处理程序代码
	theApp.dsFlag = m_DSSrvFlag.GetCheck();
}


void SmartSharingDlg::OnBnClickedRestart()
{
	// TODO: 在此添加控件通知处理程序代码
	if(CheckTransferStaus()){
		AfxMessageBox(_T("file is transfering and you can't disconnect peer"));
		return;
	}
	if (m_LocalState.GetCurrentState() == CLocalState::RECEIVINGFILE)
	{
		AfxMessageBox(_T("You are receiving files, please don't exit the window!"));
		return;
	}
	CICSSettingDlg dlg;
	trace("enter the click  start service");
	if(theApp.icsFlag == 0 && theApp.fileFlag==0 && theApp.dsFlag ==0)
	{
		AfxMessageBox(_T("Please at least select one service "));
		return;
	}

	if (1 == theApp.icsFlag)
	{
		GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Looking for available Internet connection..."));
		dlg.DoModal();
		
	}
	else if (0 == theApp.icsFlag)
	{
		theApp.stopICS();
	}
	theApp.StopSoftAP();
	theApp.StartSoftAP(theApp.fileFlag, theApp.icsFlag, theApp.dsFlag);
	//theApp.startICS();
	m_LocalState.GotoServiceRun();
	GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Start service successed."));
	trace("Start service successed.");
	trace("quit the  clickstartservice button");



}


void SmartSharingDlg::OnListSendfile()
{
	// TODO: 在此添加命令处理程序代码
	/*
	if ((m_LocalState.GetCurrentState() == CLocalState::SENDFILEWAITACK)
	|| (m_LocalState.GetCurrentState() == CLocalState::SENDFILEACKOK))
	{
			AfxMessageBox(_T("Please wait until the send file operation finished."));
			return;
	}
	if ((m_LocalState.GetCurrentState() == CLocalState::RECEIVINGFILE))
	{
		AfxMessageBox(_T("Please wait until the receiving file operation finished."));
		return;
	}
	*/
	HANDLE h_sendFile = NULL;
	int index = 0;
	EnterCriticalSection(&m_peerSection);
      index = m_Index;
    LeaveCriticalSection(&m_peerSection);
	/*
	POSITION pos = m_listConn.GetFirstSelectedItemPosition();
	if(pos==NULL)
	{
		return ;
	}
	index = (int)m_listConn.GetNextSelectedItem(pos);
	if (-1 == index)
	{
		return;
	}
	
	dst_ip = m_listConn.GetIbemText(index, 1);
	*/
	dst_ip = buttonList[index].cIP;
	toClientFlag = TRUE;
	is_dragfile = false;
	h_sendFile = CreateThread(NULL, 0, ServerSendFile, this, 0, NULL);
}


void SmartSharingDlg::OnBnClickedStopSrv()
{
	// TODO: Add your control notification handler code here
	if(CheckTransferStaus()){
		AfxMessageBox(_T("file is transfering and you can't disconnect peer"));
		return;
	}
	if (m_LocalState.GetCurrentState() == CLocalState::RECEIVINGFILE)
	{
		AfxMessageBox(_T("You are receiving files, please don't exit the window!"));
		return;
	}
	trace("enter the clickstopSrv");
	theApp.fileFlag = 0;
	theApp.icsFlag = 0;
	theApp.dsFlag = 0;
	theApp.stopICS();
	theApp.StopSoftAP();
	theApp.StartSoftAP(theApp.fileFlag, theApp.icsFlag, theApp.dsFlag);
	m_fileSrvFlag.SetCheck(0);
	m_ICSSrvFlag.SetCheck(0);
	m_DSSrvFlag.SetCheck(0);
	m_LocalState.GotoDisconnect();
	trace("quit the clickstopSrv");
}


void SmartSharingDlg::OnScanpeers()
{
	if(CheckTransferStaus()){
		AfxMessageBox(_T("file is transfering and you can't disconnect peer"));
		return;
	}
	OnBnClickedBtnscan();

}


void SmartSharingDlg::OnStartBandWidth()
{
		// TODO: 在此添加控件通知处理程序代码
	if(CheckTransferStaus()){
		AfxMessageBox(_T("file is transfering and you can't disconnect peer"));
		return;
	}
		
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
		AfxMessageBox(_T("BandWidthManager is running."));
		return;
    }
	ShellExecute(NULL, L"open", L"BandWidthManager.exe", L"", L"", SW_HIDE);
}


void SmartSharingDlg::OnStartservice()
{
	// TODO: 在此添加命令处理程序代码
		// TODO: 在此添加控件通知处理程序代码
	CICSSettingDlg dlg;
	if(CheckTransferStaus()){
		AfxMessageBox(_T("file is transfering and you can't disconnect peer"));
		return;
	}

	if(theApp.icsFlag == 0 && theApp.fileFlag==0 && theApp.dsFlag ==0)
	{
		AfxMessageBox(_T("Please at least select one service "));
		return;
	}

	if (1 == theApp.icsFlag)
	{
		GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Looking for available Internet connection..."));
		if (IDOK != dlg.DoModal())
		{
			return;
		}	
	}
	else if (0 == theApp.icsFlag)
	{
		theApp.stopICS();
	}
	theApp.StopSoftAP();
	theApp.StartSoftAP(theApp.fileFlag, theApp.icsFlag, theApp.dsFlag);
	//theApp.startICS();
	m_LocalState.GotoServiceRun();
	GetDlgItem(IDC_STATIC_HINT)->SetWindowTextW(_T("Start service successed."));

	if(theApp.dsFlag)
	{
		CString strPath = GetExePath();
		strPath +=  WINCAPSERVER;
		ShellExecute(this->m_hWnd, L"open", strPath, NULL, NULL, SW_SHOWNORMAL);
	}

	HANDLE h_handle = NULL;
	m_networkrunning = true;
	h_handle = CreateThread(NULL, 0, handleCheckNetworkLiveEvent, this, 0, NULL);
}

void SmartSharingDlg::OnScreenSharing(){
	CString strPath = GetExePath();
	strPath +=  WINPLAYER;
	CString strParam =L"rtsp://" + ip_gateway + L"//local";
	ShellExecute(this->m_hWnd, L"open", strPath, strParam, NULL, SW_SHOWNORMAL);
}

CString SmartSharingDlg::GetExePath()
{
	TCHAR pathbuffer[MAX_PATH] = {_T('\0')};
	GetModuleFileName( NULL, pathbuffer, MAX_PATH );
	PathRemoveFileSpec(pathbuffer);

	return CString( pathbuffer );
}

BOOL SmartSharingDlg::CloseProcess(LPCSTR lpstrName)
{
	WinExec(lpstrName, SW_HIDE);
/*	HANDLE hProcessHandle; 
	ULONG  nProcessID;HWND   
	TheWindow;
	TheWindow = ::FindWindow( NULL, lpstrName);//找到exe窗口句柄,lpstrName为程序的名称
	if(NULL == TheWindow)
	{	return FALSE;}
	::GetWindowThreadProcessId( TheWindow, &nProcessID );
	hProcessHandle =::OpenProcess( PROCESS_TERMINATE,FALSE, nProcessID );
	::TerminateProcess( hProcessHandle, 4 );*/
	return TRUE;
}

void SmartSharingDlg::OnStopservice()
{
	// TODO: 在此添加命令处理程序代码
	if(CheckTransferStaus()){
		AfxMessageBox(_T("file is transfering and you can't disconnect peer"));
		return;
	}
		if (m_LocalState.GetCurrentState() == CLocalState::RECEIVINGFILE)
	{
		AfxMessageBox(_T("You are receiving files, please don't exit the window!"));
		return;
	}
	CloseProcess("taskkill /im app_refs_ut_capserver.exe /f");
	m_networkrunning = false;
	theApp.fileFlag = 0;
	theApp.icsFlag = 0;
	theApp.dsFlag = 0;
	theApp.stopICS();
	theApp.StopSoftAP();
	theApp.StartSoftAP(theApp.fileFlag, theApp.icsFlag, theApp.dsFlag);
	m_fileSrvFlag.SetCheck(0);
	m_ICSSrvFlag.SetCheck(0);
	m_DSSrvFlag.SetCheck(0);
	clearbuttonlist();
	int status = 1;
	int number = 3;
	SendMessage(WM_TEST, (WPARAM)&status,(LPARAM) &number);
	m_LocalState.GotoDisconnect();
}





void SmartSharingDlg::OnBnClickedButton6()
{
	// TODO: 在此添加控件通知处理程序代码
	    EnterCriticalSection(&m_peerSection);
        m_Index = 0;
		LeaveCriticalSection(&m_peerSection);


}


void SmartSharingDlg::OnBnClickedButton7() 
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 1;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton8()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 2;
		LeaveCriticalSection(&m_peerSection);
	
}


void SmartSharingDlg::OnBnClickedButton9()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 3;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton10()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 4;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton11()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 5;
		LeaveCriticalSection(&m_peerSection);
}



void SmartSharingDlg::OnBnClickedButton12()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 6;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton13()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 7;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton14()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 8;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton15()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 9;
		LeaveCriticalSection(&m_peerSection);
}



void CAboutDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}







void SmartSharingDlg::OnBnClickedButton16()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 0;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton17()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 1;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton18()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 2;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton19()
{
	// TODO: 在此添加控件通知处理程序代码
    	EnterCriticalSection(&m_peerSection);
        m_Index = 3;
		LeaveCriticalSection(&m_peerSection);

}


void SmartSharingDlg::OnBnClickedButton20()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 4;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton21()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 5;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton22()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 6;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton23()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 7;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton24()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 8;
		LeaveCriticalSection(&m_peerSection);
}


void SmartSharingDlg::OnBnClickedButton25()
{
	// TODO: 在此添加控件通知处理程序代码
		EnterCriticalSection(&m_peerSection);
        m_Index = 9;
		LeaveCriticalSection(&m_peerSection);
}








void SmartSharingDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	UINT count;
	char filePath[512];
	//LPWSTR filePath = _T("");
	count = DragQueryFile(hDropInfo,0xFFFFFFFF,NULL,0);
	if(count)
	{

		for(UINT i=0;i<count;i++)
		{
	//LPWSTR filePath = _T("");
			int path = ::DragQueryFile(hDropInfo, i,(LPWSTR)&filePath[0], 512);
			AfxMessageBox((LPWSTR)filePath);
		}
	}


	DragFinish(hDropInfo);

	CDialog::OnDropFiles(hDropInfo);
}

void SmartSharingDlg::changeSize(int ID, int cx, int cy)
{


	CWnd *pWnd;
	pWnd = GetDlgItem(ID);     //获取控件句柄
	if(pWnd)//判断是否为空，因为对话框创建时会调用此函数，而当时控件还未创建
	{
	 CRect rect;   //获取控件变化前大小
	 pWnd->GetWindowRect(&rect);
	 ScreenToClient(&rect);//将控件大小转换为在对话框中的区域坐标
	 //　cx/m_rect.Width()为对话框在横向的变化比例
	 rect.left=rect.left*cx/m_rect.Width();/////调整控件大小
	 rect.right=rect.right*cx/m_rect.Width();
	 rect.top=rect.top*cy/m_rect.Height();
	 rect.bottom=rect.bottom*cy/m_rect.Height();
	 pWnd->MoveWindow(rect);//设置控件大小
	}

}
void SmartSharingDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	return;
	changeSize(IDC_BUTTON1, cx, cy);
	changeSize(IDC_BUTTON2, cx, cy);
	changeSize(IDC_BUTTON3, cx, cy);
	changeSize(IDC_BUTTON4, cx, cy);
	int i;
	for(i=IDC_BUTTON6;i<=IDC_BUTTON15;i++){
			changeSize(i, cx, cy);
	}
	for(i=IDC_BUTTON16;i<=IDC_BUTTON25;i++){
			changeSize(i, cx, cy);
	}
	for(i=IDC_PROGRESS1;i<=IDC_PROGRESS5;i++){
			changeSize(i, cx, cy);
	}

	GetClientRect(&m_rect);//将变化后的对话框大小设为旧大小
	

	// TODO: 在此处添加消息处理程序代码
}


BOOL SmartSharingDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此添加专用代码和/或调用基类

	return CDialog::PreCreateWindow(cs);
	
	 if( !CDialog::PreCreateWindow(cs) )
  return FALSE;
 // TODO: Modify the Window class or styles here by modifying
 //  the CREATESTRUCT cs

 cs.style&=~WS_MAXIMIZEBOX;  //禁用最大化按钮
 cs.style&=~WS_THICKFRAME;  //禁止调整窗口大小
 cs.cx=750;   //固定窗口宽度
 cs.cy=640;   //固定窗口高度（包括菜单栏等）
 

 return TRUE;


}


void SmartSharingDlg::OnListDisconnect()
{
	// TODO: Add your command handler code here
	if ((m_LocalState.GetCurrentState() == CLocalState::SENDFILEWAITACK)
	|| (m_LocalState.GetCurrentState() == CLocalState::SENDFILEACKOK))
	{
			AfxMessageBox(_T("Please wait until the send file operation finished."));
			return;
	}
	if ((m_LocalState.GetCurrentState() == CLocalState::RECEIVINGFILE))
	{
		AfxMessageBox(_T("Please wait until the receiving file operation finished."));
		return;
	}
	int index = 0;
	EnterCriticalSection(&m_peerSection);
      index = m_Index;
    LeaveCriticalSection(&m_peerSection);



	dst_ip = buttonList[index].cIP;


	if(CheckTransferStaus()){
		AfxMessageBox(_T("file is transfering and you can't disconnect peer"));
		return;
	}
	

	toClientFlag = TRUE;

	CreateThread(NULL, 0, DisConnectClient, this, 0, NULL);

}
DWORD SmartSharingDlg::DisConnectClient(LPVOID p)
{
		LPCTSTR dstIP;
		char revPkg[PKGSIZE] = {0};
		char content[CONTENTSIZE] ={0};
	    memset(revPkg, 0, PKGSIZE);
	    memset(content,0,CONTENTSIZE);
		int index;

		revPkg[0] = TRANS_DISCONNECT; // trans content
		int ret=0;

      	dstIP = ((SmartSharingDlg *)p)->dst_ip;

		ret = ((SmartSharingDlg *)p)->rcvSock.SendTo(revPkg, 1, SOCK_PORT_RECV, dstIP);	

	    WaitForSingleObject(hMutexDisconnect, INFINITE);

		/*
		int status = 1;
		int number = 3;
		index = ((SmartSharingDlg *)p)->m_Index;
		((SmartSharingDlg *)p)->buttonList[index].cIP = "";
		((SmartSharingDlg *)p)->buttonList[index].cSSID = "";
	    ((SmartSharingDlg *)p)->SendMessage(WM_TEST, (WPARAM)&status,(LPARAM) &number);
		*/
		
		return true;
}


void SmartSharingDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
}





void SmartSharingDlg::OnStnClickedPicture()
{
	// TODO: 在此添加控件通知处理程序代码
	
	CWnd *pWnd;
	if(!flag_show){

		m_button1.ShowWindow(true);
		m_button2.ShowWindow(true);
		m_button3.ShowWindow(true);
		m_button4.ShowWindow(true);
		pWnd = GetDlgItem(IDC_FILE_CHKBOX);
		pWnd->ShowWindow(true);
		pWnd = GetDlgItem(IDC_ICS_CHKBOX);
		pWnd->ShowWindow(true);
		pWnd = GetDlgItem(IDC_DS_CHKBOX);
		pWnd->ShowWindow(true);
		

		flag_show =1;
	}else{
		
		m_button1.ShowWindow(false);
		m_button2.ShowWindow(false);
		m_button3.ShowWindow(false);
		m_button4.ShowWindow(false);
		pWnd = GetDlgItem(IDC_FILE_CHKBOX);
		pWnd->ShowWindow(false);
		pWnd = GetDlgItem(IDC_ICS_CHKBOX);
		pWnd->ShowWindow(false);
		pWnd = GetDlgItem(IDC_DS_CHKBOX);
		pWnd->ShowWindow(false);
		flag_show =0;
	}
}


void SmartSharingDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
}


void SmartSharingDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
}


void SmartSharingDlg::OnLvnItemchangedListCont(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
}


void SmartSharingDlg::OnMenuSendfilecancel()
{
	// TODO: Add your command handler code here
	int index = m_Index;
	if ((m_LocalState.GetCurrentState() == CLocalState::SENDFILEACKOK)
		|| (m_LocalState.GetCurrentState() == CLocalState::RECEIVINGFILE))
	{
		/*
		EnterCriticalSection(&m_peerSection);
		is_send_file_client_cancel = true;
		LeaveCriticalSection(&m_peerSection);
		*/
		m_LocalState.GotoSendFileCancel();
		 EnterCriticalSection(&m_peerSection);
			index= m_Index;
		 LeaveCriticalSection(&m_peerSection);
		 int number = 8;
		SendMessage(WM_TEST, (WPARAM)&index,(LPARAM) &number);
	}
	else
	{
		AfxMessageBox(_T("Waitting for ack, please don't cancel."));
	}
}


void SmartSharingDlg::OnListSendfilecancel()
{
	// TODO: Add your command handler code here


	//dstIP = ((SmartSharingDlg *)p)->dst_ip;
	if (m_LocalState.GetCurrentState() == CLocalState::RECEIVINGFILE)
	{
		AfxMessageBox(_T("Now could't cacel receiving file, will do it later."));
		return;
	}
	if ((m_LocalState.GetCurrentState() == CLocalState::SENDFILEACKOK))
	{
		if(myMap[dst_ip]==NULL)
			return;

		if(!myMap[dst_ip]->send_flag){

			return;
		}
		/*
		EnterCriticalSection(&m_peerSection);
		is_send_file_server_cancel = true;
		LeaveCriticalSection(&m_peerSection);
		*/
		m_LocalState.GotoSendFileCancel();
	}
}

LRESULT SmartSharingDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class


	if(message ==WM_POWERBROADCAST ){

		if(wParam == PBT_APMRESUMEAUTOMATIC ||wParam ==PBT_APMSUSPEND){

			
					int status ;
				    int number ;

				if(m_LocalState.GetCurrentState() == SERVICERUN){

						status = 1;
						number = 3;
						SendMessage(WM_TEST, (WPARAM)&status,(LPARAM) &number);	
						int fileFlag = 0;
						int icsFlag = 0;
						int dsFlag = 0;
						fileFlag = 	theApp.fileFlag;
						icsFlag = theApp.icsFlag;
						dsFlag = theApp.dsFlag;
						OnBnClickedStopSrv();
						m_fileSrvFlag.SetCheck(fileFlag);
						m_ICSSrvFlag.SetCheck(icsFlag);
						m_DSSrvFlag.SetCheck(dsFlag);
						theApp.fileFlag = fileFlag;
						theApp.icsFlag = icsFlag;
						theApp.dsFlag = dsFlag;
						UpdateData(false);
						Sleep(1000);
						OnBnClickedRestart();

						

				}else if(m_LocalState.GetCurrentState() == CONNECTED){

						 status = 0;
						 number =4;
						SendMessage(WM_TEST, (WPARAM)&status,(LPARAM) &number);	
						m_LocalState.GotoDisconnect();
				}
				/*
				else if(m_LocalState.GetCurrentState() == CLocalState::SENDFILEACKOK){

						 status = 0;
						 number =4;
						SendMessage(WM_TEST, (WPARAM)&status,(LPARAM) &number);	
						m_LocalState.GotoSendFileCancel();
				}
				else if(m_LocalState.GetCurrentState() == CLocalState::RECEIVINGFILE){
						 status = 0;
						 number =4;
						SendMessage(WM_TEST, (WPARAM)&status,(LPARAM) &number);	
						m_LocalState.GotoDisconnect();
				}
				*/
		}
	}


	return CDialog::WindowProc(message, wParam, lParam);
}