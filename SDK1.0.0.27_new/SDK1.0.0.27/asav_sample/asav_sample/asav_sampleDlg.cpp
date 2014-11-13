// asav_sampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "asav_sample.h"
#include "asav_sampleDlg.h"
#include "log.h"
#include <wbemidl.h>
#include <Wbemcli.h>
#include <comdef.h>
#pragma comment(lib,"Wbemuuid.lib")  


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int PRIO_LVE = 0;
int PRE_LVE = 0;

int g_bDebug = 0;
// CAboutDlg dialog used for App About

int getinfo(LPTSTR buf);
HRESULT GetProcessPath(const CString& strExeFile, CString& strPath, BOOL& bFound);
CString GetDescriptionFromPath(CString IDEPath);

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// Casav_sampleDlg dialog





Casav_sampleDlg::Casav_sampleDlg(CWnd* pParent /*=NULL*/)
: CDialog(Casav_sampleDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    

 //   set_key();

	if(get_key()){

		g_bDebug = 1;
	}
	//trace("name=%s",str);

	

}

void Casav_sampleDlg::OnClose()
{

	if(m_smart_net)
		delete m_smart_net;

	m_smart_net = NULL;
	CDialog::OnClose();
	
}

void Casav_sampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LEFT, m_list_left);
	DDX_Control(pDX, IDC_LIST_RIGHT, m_list_right);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Control(pDX, IDC_LIST_BW, m_list_bw);
}

BEGIN_MESSAGE_MAP(Casav_sampleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_ADD, &Casav_sampleDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, &Casav_sampleDlg::OnBnClickedBtnDel)
	ON_BN_CLICKED(IDC_BTN_UP, &Casav_sampleDlg::OnBnClickedBtnUp)
	ON_BN_CLICKED(IDC_BTN_UPTOTOP, &Casav_sampleDlg::OnBnClickedBtnUpToTop)
	ON_BN_CLICKED(IDC_BTN_DOWN, &Casav_sampleDlg::OnBnClickedBtnDown)
	ON_BN_CLICKED(IDC_BTN_DOWNTOBOTTOM, &Casav_sampleDlg::OnBnClickedBtnDownToBottom)
	ON_BN_CLICKED(IDC_BTN_APPLY, &Casav_sampleDlg::OnBnClickedBtnApply)
	ON_BN_CLICKED(IDC_BTN_SET, &Casav_sampleDlg::OnBnClickedBtnSet)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &Casav_sampleDlg::OnBnClickedBtnClear)
	ON_BN_CLICKED(IDC_BTN_REFRESH, &Casav_sampleDlg::OnBnClickedBtnRefresh)
END_MESSAGE_MAP()


// Casav_sampleDlg message handlers

BOOL Casav_sampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	// Add SDK version at title bar
	TCHAR caption[MAX_PATH];
	//GetWindowText(caption, _countof(caption));
	// _tcscat_s(caption, _countof(caption), SDK_VERSION);
	SetWindowText(_T("Newtork Manager"));

	// Add item in right list control
	RECT rect;
	GetWindowRect(&m_rect);
	m_tab.GetWindowRect(&rect);
	m_rect_s = m_rect;
	m_rect_s.bottom = rect.top;
	//OnBnClickedBtnDetail();

	TimerLock=false;


	m_list_right.InsertColumn(LIST_PROC_NAME, _T("Process"), LVCFMT_LEFT, 150);
	m_list_right.InsertColumn(LIST_TX_PRIO, _T("Tx Priority"), LVCFMT_LEFT, 120);
	m_list_right.InsertColumn(LIST_RX_PRIO, _T("Rx Priority"), LVCFMT_LEFT, 120);
	m_list_right.SetExtendedStyle(m_list_right.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	// Add item in link list control
	if(! m_list_bw.GetItemCount())
	{
		m_pFont = m_list_bw.GetFont();
		CRect Rect(CPoint(0,0),CSize(100,500));
		m_Combo.Create(WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_HASSTRINGS | CBS_AUTOHSCROLL,Rect,this,IDC_COMBO);

		m_Combo.AddString(_T("Ultra High"));
		m_Combo.AddString(_T("High"));
		m_Combo.AddString(_T("Normal"));
		m_Combo.AddString(_T("Low"));

		m_Combo.SetFont(m_pFont);
		m_Combo.SetCurSel(2);
	}

	m_imgList.Create(1, 20, ILC_COLOR, 0, 1);
	m_list_bw.SetImageList(&m_imgList, LVSIL_SMALL);
	m_list_bw.SetExtendedStyle(m_list_bw.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_list_bw.SetBkColor(RGB(0xfe,0xFF,0xc6));
	m_list_bw.SetTextBkColor(RGB(0xfe,0xFF,0xc6));

	m_list_bw.SetGridBehaviour();



	m_list_bw.InsertColumn(LIST_PID, _T("PID"), LVCFMT_LEFT, 0);
	m_list_bw.InsertColumn(LIST_COUNT, _T("Index"), LVCFMT_LEFT,100);
	m_list_bw.InsertColumn(LIST_PROCESS, _T("Process"), LVCFMT_LEFT, 0);
	m_list_bw.InsertColumn(LIST_NAME, _T("ApplicationName"), LVCFMT_LEFT, 200);
	m_list_bw.InsertColumn(LIST_TX_BW, _T("Tx Bandwidth"), LVCFMT_LEFT, 200);
	m_list_bw.InsertColumn(LIST_RX_BW, _T("Rx Bandwidth"), LVCFMT_LEFT, 200);
	m_list_bw.InsertColumn(LIST_PRIO, _T("Priority"), LVCFMT_LEFT, 0);

	//m_list_bw.SetColumnEditor(LIST_PRIO, &Casav_sampleDlg::InitEditor, &Casav_sampleDlg::EndEditor, &m_Combo);

	//PopulateList();
	m_list_bw.RestoreState();
	PROCESS process, AllProcess[MAX_PROCESS_COUNT];
	DWORD   err;
	CString szErrMsg;
	int ProcessCount=0;

	// Get all running process 
	ProcessCount=EnumProcess(AllProcess,MAX_PROCESS_COUNT);
	if(ProcessCount<=0)
	{
		return FALSE;
	}




	RefreshListBw(AllProcess, ProcessCount);

	// Init SmartAV
	m_smart_net = new smart_net;
	if (NULL == m_smart_net) {
		return FALSE;
	}
	err = m_smart_net->init(); // 0: AR816X LOM product 1: AR815X LOM product
	if (err != ERROR_SUCCESS)
	{
		szErrMsg = get_error_msg(err);
		AfxMessageBox(szErrMsg);
		PostQuitMessage(0);
	}

	m_smart_net->lock();
	m_smart_net->auto_detect_bw_enabled(TRUE);
	m_smart_net->enable_bw_statis(TRUE); //enable TX\RX statistic

	m_smart_net->proc_seek_head();

	while (ERROR_SUCCESS == m_smart_net->get_next_proc(process.proc_name, 
		_countof(process.proc_name), &process.pid, &process.rx_prio, &process.tx_prio, 
		&process.rx_bw, &process.tx_bw))
	{
		m_list_left.AddString(process.proc_name);
		m_process_left.AddTail(process);

		//AddToListLink(process);
		UpdateListBw(process, -1);
	}
	m_smart_net->unlock();
	SetTimer(1, INTERVAL * 1000, NULL);
	SetTimer(2, INTERVAL * 5000, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Casav_sampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Casav_sampleDlg::OnPaint()
{

	CPaintDC dc(this); // device context for painting

	CBitmap bitmap;   
	bitmap.LoadBitmap(IDB_BITMAP1);
	CBrush brush; 
	brush.CreatePatternBrush(&bitmap);  
	CBrush* pOldBrush = dc.SelectObject(&brush);  
	dc.Rectangle(0,0,1000,1000);


	if (IsIconic())
	{

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Casav_sampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Casav_sampleDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(TimerLock)
		return;
	TimerLock = true;
	switch(nIDEvent)
	{
	case 1: 
		{
			m_smart_net->lock();
			UpdateAllItems();
			m_smart_net->unlock();
			/*BOOL pid_changed, net_changed;
			m_smart_net->lock();
			m_smart_net->check_proc_change(&pid_changed, &net_changed);
			if (pid_changed || net_changed) {
			UpdateProcess(pid_changed, net_changed);
			} else {
			UpdateBandwidth();
			}
			m_smart_net->unlock();*/
		}
		break;

	case 2:
		//AutoPrioSet();
		break;
	}

	TimerLock=false;

	CDialog::OnTimer(nIDEvent);
}


void Casav_sampleDlg::UpdateProcess(BOOL pid_changed, BOOL net_changed)
{
	int i, j;
	TCHAR str[32];
	DWORD pid;
	PROCESS process, process_l;
	POSITION pos, pos_l, pos_lc;
	CList<PROCESS, PROCESS&> process_list;

	//KW
	//m_list_link.DeleteAllItems();

	m_smart_net->proc_seek_head();
	while (ERROR_SUCCESS == m_smart_net->get_next_proc(process.proc_name, 
		_countof(process.proc_name), &process.pid, &process.rx_prio, &process.tx_prio,
		&process.rx_bw, &process.tx_bw))
	{
		process_list.AddTail(process);

		//AddToListLink(process);
		UpdateListBw(process, -1);
	}

	for (i = m_list_bw.GetItemCount()-1; i >= 0; i--)
	{
		m_list_bw.GetItemText(i, LIST_PID, str, _countof(str));
		pid = _ttoi(str);

		pos = process_list.GetHeadPosition();
		for (j=0; j<process_list.GetCount(); j++)
		{
			process = process_list.GetNext(pos);
			if (pid == process.pid)
			{
				break;
			}
		}
		if (j == process_list.GetCount())
		{
			//m_list_bw.DeleteItem(i);
			m_list_bw.SetItemText(i, LIST_TX_BW, _T("0"));
			m_list_bw.SetItemText(i, LIST_RX_BW, _T("0"));
			m_list_bw.SetItemText(i, LIST_PRIO, _T("None"));
		}
	}

	pos_l = m_process_right.GetTailPosition();
	for (i = (int)(m_process_right.GetCount()-1); i >= 0; i--) {
		pos_lc = pos_l;
		process_l = m_process_right.GetPrev(pos_l);

		pos = process_list.GetHeadPosition();
		for (j=0; j<process_list.GetCount(); j++) {
			process = process_list.GetNext(pos);
			if (process.pid == process_l.pid) {
				break;
			}
		}
		if (j == process_list.GetCount()) {
			m_process_right.RemoveAt(pos_lc);
			m_list_right.DeleteItem(i);
		}
	}

	pos_l = m_process_left.GetTailPosition();
	for (i = (int)(m_process_left.GetCount()-1); i >= 0; i--) {
		pos_lc = pos_l;
		process_l = m_process_left.GetPrev(pos_l);

		pos = process_list.GetHeadPosition();
		for (j=0; j<process_list.GetCount(); j++) {
			process = process_list.GetNext(pos);
			if (process.pid == process_l.pid) {
				break;
			}
		}
		if (j == process_list.GetCount()) {
			m_process_left.RemoveAt(pos_lc);
			m_list_left.DeleteString(i);
		}
	}

	pos = process_list.GetHeadPosition();
	for (j=0; j<process_list.GetCount(); j++) {
		process = process_list.GetNext(pos);

		pos_l = m_process_left.GetHeadPosition();
		for (i=0; i<m_process_left.GetCount(); i++) {
			pos_lc = pos_l;
			process_l = m_process_left.GetNext(pos_l);
			if (process.pid == process_l.pid) {
				m_process_right.SetAt(pos_lc, process);
				break;
			}
		}
		if (i < m_process_left.GetCount()) {
			continue;
		}

		pos_l = m_process_right.GetHeadPosition();

		for (i=0; i<m_process_right.GetCount(); i++) {
			pos_lc = pos_l;
			process_l = m_process_right.GetNext(pos_l);
			if (process.pid == process_l.pid) {
				m_process_right.SetAt(pos_lc, process);
				_itot_s(process.tx_prio, str, 10);
				m_list_right.SetItemText(i, LIST_TX_PRIO, str);
				GetPrioDes(process.rx_prio, str, _countof(str));
				m_list_right.SetItemText(i, LIST_RX_PRIO, str);
				break;
			}
		}

		if (i == m_process_right.GetCount()) {
			m_process_left.AddTail(process);
			m_list_left.AddString(process.proc_name);
		}
	}
}
void Casav_sampleDlg::RefreshListBw(PROCESS* process, int count)
{
	TCHAR str1[32], str2[32];
	TCHAR str3[32];
	CString prio;

	m_list_bw.DeleteAllItems();
	for(int i=0, k=0;i<count;i++)
	{
		//if there is a process with the same name, skip
		int j;
		for (j = m_list_bw.GetItemCount()-1; j>= 0; j--)
		{
			m_list_bw.GetItemText(j, LIST_PROCESS, str1, _countof(str1));
			if(_tcscmp(str1, process[i].proc_name)==0)
				break;
		}
		if(j>=0)
			continue;

		_itot_s(process[i].pid, str2, 10);
		m_list_bw.InsertItem(k, str2);

		_stprintf_s(str3, 32, _T("%d"), k);
		m_list_bw.SetItemText(k, LIST_COUNT, str3);
		m_list_bw.SetItemText(k, LIST_PROCESS, process[i].proc_name);
		m_list_bw.SetItemText(k, LIST_NAME, process[i].info);

		m_list_bw.SetItemText(k, LIST_TX_BW, _T("0"));
		m_list_bw.SetItemText(k, LIST_RX_BW, _T("0"));
		m_list_bw.SetItemText(k, LIST_PRIO, _T("None"));
		k++;
	}
}

void Casav_sampleDlg::UpdateListBw(PROCESS& process, int i)
{
	TCHAR str[32], str1[32], str2[32],str3[32];
	CString prio;
	int j;
	if (i < 0) {
		/* for (j=0; j<m_list_bw.GetItemCount(); j++) {
		m_list_bw.GetItemText(j, LIST_PID, str1, _countof(str1));
		pid = _ttoi(str1);
		if (pid == process.pid) {
		i = j;
		break;
		}
		}
		if (j == m_list_bw.GetItemCount()) {
		//i = 0;
		//_itot_s(process.pid, str1, 10);
		//m_list_bw.InsertItem(0, str1);
		//m_list_bw.SetItemText(0, LIST_PROCESS, process.proc_name);
		//skip if not found
		return;
		}*/
		//update based on the proc name
		for (j=0; j<m_list_bw.GetItemCount(); j++) {
			m_list_bw.GetItemText(j, LIST_PROCESS, str3, _countof(str3));
			if(_tcscmp(str3, process.proc_name)==0){
				i=j;
				break;
			}
		}

		if (j == m_list_bw.GetItemCount()) {
			/*i = 0;
			_itot_s(process.pid, str1, 10);
			m_list_bw.InsertItem(0, str1);
			m_list_bw.SetItemText(0, LIST_PROCESS, process.proc_name);*/
			//skip if not found
			return;
		}
	}
	_itot_s(process.pid, str, 10);
	m_list_bw.SetItemText(i, LIST_PID,str);

	ConvertBpsToStr(process.tx_bw, str1, _countof(str1));
	m_list_bw.GetItemText(i, LIST_TX_BW, str2, _countof(str2));
	if (_tcscmp(str1, str2)) {
		m_list_bw.SetItemText(i, LIST_TX_BW, str1);
	}

	ConvertBpsToStr(process.rx_bw, str1, _countof(str1));
	m_list_bw.GetItemText(i, LIST_RX_BW, str2, _countof(str2));
	if (_tcscmp(str1, str2)) {
		m_list_bw.SetItemText(i, LIST_RX_BW, str1);
	}

	/*prio = m_list_bw.GetItemText(i, LIST_PRIO);
	if (prio.GetLength() == 0) {
	m_list_bw.SetItemText(i, LIST_PRIO, _T("Normal"));
	}*/
	if(process.rx_prio != 0)
	{
		_itot_s(process.rx_prio, str, 10);
		m_list_bw.SetItemText(i, LIST_PRIO,str);
	}
}

void Casav_sampleDlg::ConvertBpsToStr(DWORD Bps, TCHAR *str, int size)
{
	float bw;
	if (Bps >= 1000000) {
		bw = (float)Bps / 1000000;
		_stprintf_s(str, size, _T("%.2f"), bw);
		_tcscat_s(str, size, _T(" MBps"));
	} else if (Bps >= 1000) {
		bw = (float)Bps / 1000;
		_stprintf_s(str, size, _T("%.2f"), bw);
		_tcscat_s(str, size, _T(" KBps"));
	} else {
		_stprintf_s(str, size, _T("%d"), Bps);
		_tcscat_s(str, size, _T(" Bps"));
	}
}

void Casav_sampleDlg::OnBnClickedBtnAdd()
{
	// TODO: Add your control notification handler code here
	int i, j;
	POSITION pos;
	PROCESS process;
	i = m_list_left.GetCurSel();
	if (i != LB_ERR) {
		pos = m_process_left.FindIndex(i);
		process = m_process_left.GetAt(pos);
		m_process_left.RemoveAt(pos);
		m_list_left.DeleteString(i);

		m_process_right.AddTail(process);
		j = m_list_right.GetItemCount();
		InsertToListRight(j, process);
	}
}

void Casav_sampleDlg::InsertToListRight(int i, PROCESS& process)
{
	TCHAR str[32];

	m_list_right.InsertItem(i, process.proc_name);
	_itot_s(process.tx_prio, str, 10);
	m_list_right.SetItemText(i, LIST_TX_PRIO, str);
	GetPrioDes(process.rx_prio, str, _countof(str));
	m_list_right.SetItemText(i, LIST_RX_PRIO, str);
}

void Casav_sampleDlg::GetPrioDes(ULONG prio, TCHAR *str, int size)
{

	switch(prio)
	{
	case ALL_LOW_PRIO:
		_tcscpy_s(str, size, _T("All Low Priority"));
		break;
	case PART_HIGH_PRIO:
		_tcscpy_s(str, size, _T("Part High Priority"));
		break;
	case FULL_HIGH_PRIO:
		_tcscpy_s(str, size, _T("Full High Priority"));
		break;
	default:
		_tcscpy_s(str, size, _T(""));
	}
}
void Casav_sampleDlg::OnBnClickedBtnDel()
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	int i;
	POSITION pos;
	PROCESS process;
	pos = m_list_right.GetFirstSelectedItemPosition();
	if (pos) {
		i = m_list_right.GetNextSelectedItem(pos);
		m_list_right.DeleteItem(i);
		pos = m_process_right.FindIndex(i);
		process = m_process_right.GetAt(pos);
		m_process_right.RemoveAt(pos);

		m_process_left.AddTail(process);
		m_list_left.AddString(process.proc_name);
	}
}

void Casav_sampleDlg::OnBnClickedBtnUp()
{
	// TODO: Add your control notification handler code here
	trace("enter the up button");
	int i;
	POSITION pos;
	CString pid, proc_name,proc_exe, tx_bw, rx_bw, priority;

	pos = m_list_bw.GetFirstSelectedItemPosition();
	if (pos) {
		i = m_list_bw.GetNextSelectedItem(pos);
		if (i > 0) {
			pid = m_list_bw.GetItemText(i-1, LIST_PID);
			proc_name = m_list_bw.GetItemText(i-1, LIST_PROCESS);
			proc_exe =  m_list_bw.GetItemText(i-1, LIST_NAME);
			tx_bw = m_list_bw.GetItemText(i-1, LIST_TX_BW);	
			rx_bw = m_list_bw.GetItemText(i-1, LIST_RX_BW);
			priority = m_list_bw.GetItemText(i-1, LIST_PRIO);

			m_list_bw.SetItemText(i-1, LIST_PID, m_list_bw.GetItemText(i, LIST_PID));
			m_list_bw.SetItemText(i-1, LIST_PROCESS, m_list_bw.GetItemText(i, LIST_PROCESS));
			m_list_bw.SetItemText(i-1, LIST_NAME, m_list_bw.GetItemText(i, LIST_NAME));
			m_list_bw.SetItemText(i-1, LIST_TX_BW, m_list_bw.GetItemText(i, LIST_TX_BW));
			m_list_bw.SetItemText(i-1, LIST_RX_BW, m_list_bw.GetItemText(i, LIST_RX_BW));
			m_list_bw.SetItemText(i-1, LIST_PRIO, m_list_bw.GetItemText(i, LIST_PRIO));

			m_list_bw.SetItemText(i, LIST_PID, pid);
			m_list_bw.SetItemText(i, LIST_PROCESS, proc_name);
			m_list_bw.SetItemText(i, LIST_NAME, proc_exe);
			m_list_bw.SetItemText(i, LIST_TX_BW, tx_bw);
			m_list_bw.SetItemText(i, LIST_RX_BW, rx_bw);
			m_list_bw.SetItemText(i, LIST_PRIO, priority);

			m_list_bw.SetItemState(i-1, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_list_bw.SetItemState(i, 0, -1);
		}
		m_list_bw.SetFocus();
		m_list_bw.PostMessage(WM_VSCROLL,  SB_LINEUP , 0);
	}
		trace("leave the up button");
}

void Casav_sampleDlg::OnBnClickedBtnDown()
{
	// TODO: Add your control notification handler code here
	trace("enter the down button");
	int i;
	POSITION pos;
	CString pid, proc_name,proc_exe, tx_bw, rx_bw, priority;

	pos = m_list_bw.GetFirstSelectedItemPosition();
	if (pos) {
		i = m_list_bw.GetNextSelectedItem(pos);
		if (i < (m_list_bw.GetItemCount() - 1)) {
			pid = m_list_bw.GetItemText(i+1, LIST_PID);
			proc_name = m_list_bw.GetItemText(i+1, LIST_PROCESS);
			proc_exe =  m_list_bw.GetItemText(i+1, LIST_NAME);
			tx_bw = m_list_bw.GetItemText(i+1, LIST_TX_BW);	
			rx_bw = m_list_bw.GetItemText(i+1, LIST_RX_BW);
			priority = m_list_bw.GetItemText(i+1, LIST_PRIO);

			m_list_bw.SetItemText(i+1, LIST_PID, m_list_bw.GetItemText(i, LIST_PID));
			m_list_bw.SetItemText(i+1, LIST_PROCESS, m_list_bw.GetItemText(i, LIST_PROCESS));
			m_list_bw.SetItemText(i+1, LIST_NAME, m_list_bw.GetItemText(i, LIST_NAME));
			m_list_bw.SetItemText(i+1, LIST_TX_BW, m_list_bw.GetItemText(i, LIST_TX_BW));
			m_list_bw.SetItemText(i+1, LIST_RX_BW, m_list_bw.GetItemText(i, LIST_RX_BW));
			m_list_bw.SetItemText(i+1, LIST_PRIO, m_list_bw.GetItemText(i, LIST_PRIO));

			m_list_bw.SetItemText(i, LIST_PID, pid);
			m_list_bw.SetItemText(i, LIST_PROCESS, proc_name);
			m_list_bw.SetItemText(i, LIST_NAME, proc_name);
			m_list_bw.SetItemText(i, LIST_TX_BW, tx_bw);
			m_list_bw.SetItemText(i, LIST_RX_BW, rx_bw);
			m_list_bw.SetItemText(i, LIST_PRIO, priority);

			m_list_bw.SetItemState(i+1, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_list_bw.SetItemState(i, 0, -1);
		}
		m_list_bw.SetFocus();
			m_list_bw.PostMessage(WM_VSCROLL,  SB_LINEDOWN , 0);
	}
	trace("leave the down button");
}


void Casav_sampleDlg::OnBnClickedBtnUpToTop()
{
	int i;
	POSITION pos;
	CString pid, proc_name,proc_exe, tx_bw, rx_bw, priority;

	trace("enter the UptoTop button");

	pos = m_list_bw.GetFirstSelectedItemPosition();
	if (pos) {
		i = m_list_bw.GetNextSelectedItem(pos);
		if (i > 0) {
			pid = m_list_bw.GetItemText(i, LIST_PID);
			proc_name = m_list_bw.GetItemText(i, LIST_PROCESS);
			proc_exe = m_list_bw.GetItemText(i, LIST_NAME);
			tx_bw = m_list_bw.GetItemText(i, LIST_TX_BW);	
			rx_bw = m_list_bw.GetItemText(i, LIST_RX_BW);
			priority = m_list_bw.GetItemText(i, LIST_PRIO);

			for(int j=i-1;j>=0;j--)
			{
				m_list_bw.SetItemText(j+1, LIST_PID, m_list_bw.GetItemText(j, LIST_PID));
				m_list_bw.SetItemText(j+1, LIST_PROCESS, m_list_bw.GetItemText(j, LIST_PROCESS));
				m_list_bw.SetItemText(j+1, LIST_NAME, m_list_bw.GetItemText(j, LIST_NAME));
				m_list_bw.SetItemText(j+1, LIST_TX_BW, m_list_bw.GetItemText(j, LIST_TX_BW));
				m_list_bw.SetItemText(j+1, LIST_RX_BW, m_list_bw.GetItemText(j, LIST_RX_BW));	
				m_list_bw.SetItemText(j+1, LIST_PRIO, m_list_bw.GetItemText(j, LIST_PRIO));
			}

			m_list_bw.SetItemText(0, LIST_PID, pid);
			m_list_bw.SetItemText(0, LIST_PROCESS, proc_name);
			m_list_bw.SetItemText(0, LIST_NAME, proc_exe);
			m_list_bw.SetItemText(0, LIST_TX_BW, tx_bw);
			m_list_bw.SetItemText(0, LIST_RX_BW, rx_bw);
			m_list_bw.SetItemText(0, LIST_PRIO, priority);

			m_list_bw.SetItemState(0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_list_bw.SetItemState(i, 0, -1);
		}
		m_list_bw.SetFocus();
		m_list_bw.PostMessage(WM_VSCROLL, SB_TOP, 0);
	}

	trace("leave  the UptoTop button");
}

void Casav_sampleDlg::OnBnClickedBtnDownToBottom()
{
	int i;
	POSITION pos;
	CString pid, proc_name,proc_exe, tx_bw, rx_bw, priority;

	trace("enter the DownToButtom");

	pos = m_list_bw.GetFirstSelectedItemPosition();
	if (pos) {
		i = m_list_bw.GetNextSelectedItem(pos);
		if (i < (m_list_bw.GetItemCount() - 1)) {
			pid = m_list_bw.GetItemText(i, LIST_PID);
			proc_name = m_list_bw.GetItemText(i, LIST_PROCESS);
			proc_exe = m_list_bw.GetItemText(i, LIST_NAME);
			tx_bw = m_list_bw.GetItemText(i, LIST_TX_BW);	
			rx_bw = m_list_bw.GetItemText(i, LIST_RX_BW);
			priority = m_list_bw.GetItemText(i, LIST_PRIO);

			for(int j=i+1;j<=(m_list_bw.GetItemCount() - 1);j++)
			{
				m_list_bw.SetItemText(j-1, LIST_PID, m_list_bw.GetItemText(j, LIST_PID));
				m_list_bw.SetItemText(j-1, LIST_PROCESS, m_list_bw.GetItemText(j, LIST_PROCESS));
				m_list_bw.SetItemText(j-1, LIST_NAME, m_list_bw.GetItemText(j, LIST_NAME));
				m_list_bw.SetItemText(j-1, LIST_TX_BW, m_list_bw.GetItemText(j, LIST_TX_BW));
				m_list_bw.SetItemText(j-1, LIST_RX_BW, m_list_bw.GetItemText(j, LIST_RX_BW));	
				m_list_bw.SetItemText(j-1, LIST_PRIO, m_list_bw.GetItemText(j, LIST_PRIO));
			}

			m_list_bw.SetItemText((m_list_bw.GetItemCount() - 1), LIST_PID, pid);
			m_list_bw.SetItemText((m_list_bw.GetItemCount() - 1), LIST_PROCESS, proc_name);
			m_list_bw.SetItemText((m_list_bw.GetItemCount() - 1), LIST_NAME, proc_exe);
			m_list_bw.SetItemText((m_list_bw.GetItemCount() - 1), LIST_TX_BW, tx_bw);
			m_list_bw.SetItemText((m_list_bw.GetItemCount() - 1), LIST_RX_BW, rx_bw);
			m_list_bw.SetItemText((m_list_bw.GetItemCount() - 1), LIST_PRIO, priority);

			m_list_bw.SetItemState((m_list_bw.GetItemCount() - 1), LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_list_bw.SetItemState(i, 0, -1);
		}
		m_list_bw.SetFocus();
		m_list_bw.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
	}

	trace("leave  the DownToButtom");
}

void Casav_sampleDlg::OnBnClickedBtnApply()
{
	// TODO: Add your control notification handler code here
	int i;
	POSITION pos;
	PROCESS process;

	

	m_smart_net->clearall_prio();
	pos = m_process_right.GetHeadPosition();
	for (i=1; i<=m_process_right.GetCount(); i++) {
		process = m_process_right.GetNext(pos);
		m_smart_net->set_proc_prio(process.proc_name, i);
	}
	m_smart_net->apply_prio_list();
	OnTimer(1);
}

void Casav_sampleDlg::OnBnClickedBtnSet()
{
	// TODO: Add your control notification handler code here
	//PRIO_LVE = 1;

	//AutoPrioSet();
	//OnTimer(2);
	TCHAR str[32];
	int i=0, j=0;
	CString  proc_name, rx;
	trace("enter priority button");

	m_smart_net->auto_detect_bw_enabled(TRUE);

	m_smart_net->clearall_prio();

	/* for (i=0, j=0; i<m_list_bw.GetItemCount() && j < MAX_PRIORITY_LEVEL; i++) {
	rx = m_list_bw.GetItemText(i, LIST_RX_BW);
	if(rx=="0")//the process has no traffic
	continue;

	proc_name = m_list_bw.GetItemText(i, LIST_PROCESS);
	if (proc_name.GetLength() > 0)
	{
	m_smart_net->set_proc_prio(proc_name, j+1);
	_itot_s(j+1, str, 10);
	m_list_bw.SetItemText(i, LIST_PRIO, str);//Do not set the value here, rely on the timer to refresh the priority from the driver
	j++;
	}
	}*/
	for (i=0; i<m_list_bw.GetItemCount() && i < MAX_PRIORITY_LEVEL; i++) {

		proc_name = m_list_bw.GetItemText(i, LIST_PROCESS);
		if (proc_name.GetLength() > 0)
		{
			m_smart_net->set_proc_prio(proc_name, i+1);
			_itot_s(i+1, str, 10);
			m_list_bw.SetItemText(i, LIST_PRIO, str);//the timer will refresh the priority from the driver
		}
	}
	m_smart_net->apply_prio_list();

	trace("leave priority button");
}

void Casav_sampleDlg::OnBnClickedBtnClear()
{
	trace("enter the clear priority button");
	m_smart_net->clearall_prio();
	m_smart_net->apply_prio_list();
	for (int i=0; i<m_list_bw.GetItemCount() && i < MAX_PRIORITY_LEVEL; i++) {
		m_list_bw.SetItemText(i, LIST_PRIO, _T("None"));//the timer will refresh the priority from the driver	
	}
	trace("leave the clear priority button");
}

void Casav_sampleDlg::OnBnClickedBtnRefresh()
{
	PROCESS AllProcess[MAX_PROCESS_COUNT];
	int ProcessCount=0;

	trace("enter refresh button");
	// Get all running process 
	ProcessCount=EnumProcess(AllProcess,MAX_PROCESS_COUNT);
	if(ProcessCount<=0)
	{
		return ;
	}
	RefreshListBw(AllProcess, ProcessCount);
	trace("leave refresh button");
}
void Casav_sampleDlg::AutoPrioSet()
{
	// TODO: Add your control notification handler code here
	int      i;
	CString  proc_name;
	CString  level;

	if (PRIO_LVE == 0)
		return;

	if (m_smart_net->get_rx_available_bw() < 1000 && PRIO_LVE > 1)
	{
		PRIO_LVE--;
	}

	level.Format(_T("%d"), PRIO_LVE);
	//AfxMessageBox(level);

	m_smart_net->clearall_prio();

	switch(PRIO_LVE)
	{
	case 1:
		{
			for (i=0; i<=m_list_bw.GetItemCount(); i++) {
				if (_tcscmp(m_list_bw.GetItemText(i, LIST_PRIO), _T("Ultra High")) == 0) {
					proc_name = m_list_bw.GetItemText(i, LIST_PROCESS);
					if (proc_name.GetLength() > 0)
						m_smart_net->set_proc_prio(m_list_bw.GetItemText(i, LIST_PROCESS), i+1);
				}
			}
			m_smart_net->apply_prio_list();
		}
		break;
	case 2:
		{
			for (i=0; i<=m_list_bw.GetItemCount(); i++) {
				if ((_tcscmp(m_list_bw.GetItemText(i, LIST_PRIO), _T("Ultra High")) == 0) ||
					(_tcscmp(m_list_bw.GetItemText(i, LIST_PRIO), _T("High")) == 0)) {
						proc_name = m_list_bw.GetItemText(i, LIST_PROCESS);
						if (proc_name.GetLength() > 0)
							m_smart_net->set_proc_prio(m_list_bw.GetItemText(i, LIST_PROCESS), i+1);
				}
			}
			m_smart_net->apply_prio_list();
		}
		break;
	case 3:
		{
			for (i=0; i<=m_list_bw.GetItemCount(); i++) {
				if (_tcscmp(m_list_bw.GetItemText(i, LIST_PRIO), _T("Low")) != 0) {
					proc_name = m_list_bw.GetItemText(i, LIST_PROCESS);
					if (proc_name.GetLength() > 0)
						m_smart_net->set_proc_prio(m_list_bw.GetItemText(i, LIST_PROCESS), i+1);
				}
			}
			m_smart_net->apply_prio_list();
		}
		break;
	case 4:
		{
			for (i=0; i<=m_list_bw.GetItemCount(); i++) {
				proc_name = m_list_bw.GetItemText(i, LIST_PROCESS);
				if (proc_name.GetLength() > 0)
					m_smart_net->set_proc_prio(m_list_bw.GetItemText(i, LIST_PROCESS), i+1);
			}
			m_smart_net->apply_prio_list();
		}
		break;
	}

	if (PRIO_LVE < 4)
		PRIO_LVE++;
}

void Casav_sampleDlg::UpdateAllItems()
{
	int i=0, j=0;
	TCHAR str[64];
	DWORD pid;
	UINT32 tx_bw, rx_bw;
	PROCESS process;
	POSITION pos;
	CList<PROCESS, PROCESS&> process_list;

	m_smart_net->proc_seek_head();

	while (ERROR_SUCCESS == m_smart_net->get_next_proc(process.proc_name, 
		_countof(process.proc_name), &process.pid, &process.rx_prio, &process.tx_prio, 
		&process.rx_bw, &process.tx_bw))
	{
		process_list.AddTail(process);
		UpdateListBw(process, -1);
	}
	for (i = m_list_bw.GetItemCount()-1; i >= 0; i--)
	{
		m_list_bw.GetItemText(i, LIST_PROCESS, str, _countof(str));

		pos = process_list.GetHeadPosition();
		for (j=0; j<process_list.GetCount(); j++)
		{
			process = process_list.GetNext(pos);
			if(_tcscmp(str, process.proc_name)==0){

				break;
			}
		}
		if (j == process_list.GetCount())
		{
			//m_list_bw.DeleteItem(i);
			m_list_bw.SetItemText(i, LIST_TX_BW, _T("0"));
			m_list_bw.SetItemText(i, LIST_RX_BW, _T("0"));
			m_list_bw.SetItemText(i, LIST_PRIO, _T("None"));
		}
	}


}

void Casav_sampleDlg::UpdateBandwidth()
{
	int i;
	TCHAR str[64];
	DWORD pid;
	UINT32 tx_bw, rx_bw;
	PROCESS process;

	for (i=0; i<m_list_bw.GetItemCount(); i++) {
		m_list_bw.GetItemText(i, LIST_PID, str, _countof(str));
		pid = _ttoi(str);
		m_smart_net->get_bw_statis_by_pid(pid, &rx_bw, &tx_bw);
		process.pid = pid;
		process.tx_bw = tx_bw;
		process.rx_bw = rx_bw;
		UpdateListBw(process, i);
	}
}
BOOL Casav_sampleDlg::InitEditor(CWnd** pWnd, int nRow, int nColumn, CString &strSubItemText, DWORD_PTR dwItemData, void* pThis, BOOL bUpdate)
{
	ASSERT(*pWnd);
	switch(nColumn)
	{
	case 4:
		{
			CComboBox* pCmb = reinterpret_cast<CComboBox*>(*pWnd);
			pCmb->SelectString(0, strSubItemText);
		}
		break;
	case 3:
		{
			CDateTimeCtrl* pDTC = reinterpret_cast<CDateTimeCtrl*>(*pWnd);
			COleDateTime dt;
			if(dt.ParseDateTime(strSubItemText))pDTC->SetTime(dt);
		}
		break;
	}

	return TRUE;
}

BOOL Casav_sampleDlg::EndEditor(CWnd** pWnd, int nRow, int nColumn, CString &strSubItemText, DWORD_PTR dwItemData, void* pThis, BOOL bUpdate)
{
	ASSERT(pWnd);
	switch(nColumn)
	{
	case 4:
		{
			CComboBox* pCmb = reinterpret_cast<CComboBox*>(*pWnd);
			int index = pCmb->GetCurSel();
			if(index >= 0) pCmb->GetLBText(index, strSubItemText);
		}
		break;
	case 3:
		{
			CDateTimeCtrl* pDTC = reinterpret_cast<CDateTimeCtrl*>(*pWnd);
			COleDateTime dt;
			pDTC->GetTime(dt);
			strSubItemText = dt.Format();
		}
		break;
	}

	return TRUE;
}

void Casav_sampleDlg::PopulateList()
{
	int nIndex = -1;

	m_list_bw.DeleteAllItems();

	for(int i = 0;i < 10;++i)
	{
		CString sTemp;

		sTemp.Format(_T("%d"),i * 100);
		nIndex = m_list_bw.InsertItem(i, sTemp);


		m_list_bw.SetItemText(i, LIST_PRIO, _T("Normal"));
	}
}

void WriteLog(LPCTSTR logName, CString msg)
{
    try
    {
        //设置文件的打开参数
        CStdioFile outFile(logName, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite | CFile::typeText);
        CString msLine;
        CTime tt = CTime::GetCurrentTime();

        //作为Log文件，经常要给每条Log打时间戳，时间格式可自由定义，
        //这里的格式如：2010-June-10 Thursday, 15:58:12
        msLine += msg;
        msLine += "\n";

        //在文件末尾插入新纪录
        outFile.SeekToEnd();
        outFile.WriteString( msLine );
        outFile.Close();
    }
    catch(CFileException *fx)
    {
        fx->Delete();
    }
}


int Casav_sampleDlg::EnumProcess(PROCESS * ProcessList, long MaxSize)
{
	CString logName = CString("C:\\aa.log");
	HANDLE hProcess;
	int index=0;
	BOOL bProcessEnd=FALSE;
	PROCESSENTRY32 Info, *pInfo;
	DWORD err;
	CString strModuleName =_T("");
	TCHAR szModuleName[MAX_PATH];
	
  //  _tmemset(szModuleName,0,MAX_PATH);



	Info.dwSize=sizeof(PROCESSENTRY32);
	pInfo=&Info;
	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if( INVALID_HANDLE_VALUE == hProcess)
		return -1;


	bProcessEnd=Process32First(hProcess,pInfo);
	err=GetLastError();
	while(bProcessEnd)
	{
			_tcsncpy(ProcessList[index].proc_name, pInfo->szExeFile,_tcsnlen(pInfo->szExeFile,MAX_PATH)+1);//_tcsnlen(pinfo->szExeFile,MAX_PATH),


			ProcessList[index].pid = pInfo->th32ProcessID;

		
		
			CString strPath;
			BOOL bFound = false;
			CString filepath;
			CString desc;
			int len ;
			int result =0;
			CString strExeFile = CString(ProcessList[index].proc_name);
			if(ProcessList[index].pid !=4 && ProcessList[index].pid !=0){

				GetProcessPath(strExeFile,  strPath, bFound);
				if(bFound){
				//	filepath = strPath;
				//desc = GetDescriptionFromPath(strPath);
					_tcsncpy_s(szModuleName, strPath.GetBuffer(strPath.GetLength()),strPath.GetLength());
					szModuleName[strPath.GetLength()]='\0';
					result = getinfo(szModuleName);
					if(result ==0){
							
						_tcsncpy(ProcessList[index].info, ProcessList[index].proc_name,MAX_PATH);
					}else{
						 len = _tcslen(szModuleName);
						_tcsncpy(ProcessList[index].info, szModuleName,len);
						ProcessList[index].info[len] ='\0';
					}

				}else{

					_tcsncpy(ProcessList[index].info, ProcessList[index].proc_name,MAX_PATH);
				}

	
			}else if(ProcessList[index].pid==0){
	
						_tcscpy(ProcessList[index].info, _T("System Idle Process"));
						ProcessList[index].info[_tcslen(_T("System Idle Process"))]='\0';
				
				

			}else if(ProcessList[index].pid==4){
		
						_tcscpy(ProcessList[index].info, _T("System"));
					ProcessList[index].info[_tcslen(_T("System"))]='\0';
			}

			
	

			/*
		GetProcessName(ProcessList[index].proc_name,pInfo->th32ProcessID,szModuleName,MAX_PATH);

		if(CString(_T("unknown(OpenProcess error)")) ==CString(szModuleName)){

			_tcsncpy(ProcessList[index].info, 	ProcessList[index].proc_name,MAX_PATH);

		}else if(CString( _T("unknown(GetModuleBaseName error)")) ==CString(szModuleName)){


			_tcsncpy(ProcessList[index].info, 	ProcessList[index].proc_name,MAX_PATH);

		}
		else{
			_tcsncpy(ProcessList[index].info, szModuleName,MAX_PATH);


		}
		*/
		



		bProcessEnd=Process32Next(hProcess, pInfo);   
		index++;
		if(index >= MaxSize) 
			break;
	}





	CloseHandle(hProcess);
	return index;
}

void Casav_sampleDlg::GetProcessName(LPTSTR name,DWORD processid,LPTSTR buf,int len)
{
	try
	{
		CString strPath;
		CString strExeFile(name);
		CString desc;
		 BOOL bFound = false;
		//make   sure   buf   is   valid   and   long   enough
		if(buf==NULL||len <=0)
			return;
		ZeroMemory(buf, len);
		if(processid == 4)
		{
			_tcscpy(buf, _T("System"));
			return;
		}
		if(processid==0)
		{
			_tcscpy(buf, _T("System Idle Process"));
			return;
		}
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,false,processid);
		if(hProcess ==NULL)
		{
			//_tcscpy(buf, _T("unknown(OpenProcess error)"));
			_tcscpy(buf, name);
			getinfo(buf);
			/*
			GetProcessPath(strExeFile,  strPath, bFound);
			desc = GetDescriptionFromPath(strExeFile);
			_tcscpy(buf, desc.GetBuffer(desc.GetLength()));
			*/
			return;
		}
		HMODULE   hModule;
		DWORD   cbReturned;
		BOOL   bret=EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbReturned);
		if(bret)
		{
			DWORD   dwret=GetModuleFileNameEx(hProcess, hModule, buf, len);

			GetModuleBaseName(hProcess,hModule,buf,len);
			/*
			GetProcessPath(strExeFile,  strPath, bFound);
			desc = GetDescriptionFromPath(strPath);
			_tcscpy(buf, desc.GetBuffer(desc.GetLength()));
			*/
		}
		else{

			//_tcscpy(buf, _T("unknown(GetModuleBaseName error)"));

			_tcscpy(buf, name);
			getinfo(buf);


		}
		CloseHandle(hProcess)   ;
		// GetProcessName2(processid,buf);
	}
	catch(...){

	}

}

HRESULT GetProcessPath(const CString& strExeFile, CString& strPath, BOOL& bFound)
{
	HRESULT hr;
	
	hr =  CoInitializeEx(0, COINIT_MULTITHREADED); 
	if (FAILED(hr))
    {
		AfxMessageBox(_T("Failed to initialize COM library. Error code = 0x"));
		return hr;                  // Program has failed.
    }

	bFound = FALSE;
	// initialize the IWbemLocator interface
	IWbemLocator *pLoc = NULL;
	hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
	if(FAILED(hr))
	{
		CoUninitialize();
		return hr;
	}
	// create a connection to WMI namespace.
	IWbemServices *pSvc = NULL;
	hr = pLoc->ConnectServer(bstr_t("ROOT\\CIMV2"), 
		NULL, NULL, 0, NULL, 0, 0, &pSvc);
	if(FAILED(hr))
	{
		pLoc->Release();
		CoUninitialize();
		return hr;
	}
	// set security levels on the proxy
	hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
		NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if(FAILED(hr))
	{
		pSvc->Release();
		pLoc->Release();     
		CoUninitialize();
		return hr;
	}
	// do query
	IEnumWbemClassObject* pEnum = NULL;
	hr = pSvc->ExecQuery(bstr_t("WQL"), 
		bstr_t("SELECT Name, ExecutablePath FROM Win32_Process"),
		WBEM_FLAG_FORWARD_ONLY|WBEM_FLAG_RETURN_IMMEDIATELY, 
		NULL,
		&pEnum);

	if(FAILED(hr))
	{
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return hr;
	}
	// get each processes enumeration
	IWbemClassObject *pclsObj;
	ULONG uReturn = 0;
	while(pEnum)
	{
		pEnum->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
		if(0 == uReturn)
			break;

		// get properties values
		VARIANT vtName;
		int result = pclsObj->Get(L"Name", 0, &vtName, 0, 0);
		int dwError;
		dwError = GetLastError();
		if(!strExeFile.CompareNoCase(vtName.bstrVal))
		{
			VARIANT vtExecutablePath;
			dwError = GetLastError();
			if(CString(vtName)==CString("smss.exe")){
				bFound = FALSE;
				break;
			}
			pclsObj->Get(L"ExecutablePath", 0, &vtExecutablePath, 0, 0);
			if(vtExecutablePath.bstrVal ==NULL)
			{
				strPath = "";
			}
			dwError = GetLastError();
			printf("dwError=%d\n",dwError);
			strPath =  COLE2T(vtExecutablePath.bstrVal);
			
			VariantClear(&vtExecutablePath);
			bFound = TRUE; // successfully found
			break;
		}
		VariantClear(&vtName);
	}
	// cleanup
	pclsObj->Release();
	pEnum->Release();
	pSvc->Release();
	pLoc->Release();  
	CoUninitialize();
	return hr;
}
CString GetDescriptionFromPath(CString IDEPath)
{
    DWORD   dwLen = 0;
    TCHAR*  lpData = NULL;
	CString Ret;
	LPVOID  lpBuffer = NULL;
    UINT    uLen = 0;
	UINT    i;
	BOOL    bSuccess;
	struct  LANGANDCODEPAGE {
		      WORD wLanguage;
		      WORD wCodePage;
	        } *lpTranslate;

    dwLen = GetFileVersionInfoSize(IDEPath, 0);
    if (0 == dwLen)
        return CString(_T(""));

    lpData =new TCHAR [dwLen+1];

    if(!GetFileVersionInfo(IDEPath, 0, dwLen, lpData))
    {
        delete lpData;
        return CString(_T(""));
    }
	
	// Read the list of languages and code pages.
	VerQueryValue(lpData, 
				  TEXT("\\VarFileInfo\\Translation"),
				  (LPVOID*)&lpTranslate,
				  &uLen);
	for( i = 0; i < (uLen/sizeof(struct LANGANDCODEPAGE)); i++ )
	{
		CString Trans;
		
		Trans.Format(_T("\\StringFileInfo\\%04x%04x\\FileDescription"), lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
		
		// Retrieve file description for language and code page "i". 
		bSuccess = VerQueryValue(lpData, (LPWSTR)(LPCTSTR)Trans, &lpBuffer, &uLen);

		if (bSuccess)
			Ret = (TCHAR*)lpBuffer;
	}

    delete [] lpData;
    return Ret;
}

int getinfo(LPTSTR buf)
{

#define SWAPWORDS(X) ( (X<<16) | (X>>16) )
	int  dwSize =  MAX_PATH;
	TCHAR *lpBuffer;
	DWORD dwHandle;
	DWORD dwError;
	UINT dwUint;
	TCHAR *Buffer;
	

	

	if(!(dwSize = GetFileVersionInfoSize(buf, &dwHandle) ) )
	{
		CString  name = CString(buf);
		int pos = name.ReverseFind('\\');
		CString path = name.Mid(pos+1,name.GetLength()-pos);
		 _tcscpy(buf, path.GetBuffer(0));
		dwError = GetLastError();
		if (dwError == 2){
			// 2 is file no found error
			printf("exe is not found");
		}

		else if (dwError == 1812){
			// 1812 means no resource section information, very unlikely case
			printf("exel does contain resource section");
		}
		else{
			printf("GetFileVersionInfoSize failed: %d", GetLastError());

		}
	
		return 0;
	}
	

	lpBuffer = new TCHAR [dwSize]; 
	if (!GetFileVersionInfo (buf, 0, 
		dwSize, (LPVOID) lpBuffer) )
	{
		printf("GetFileVersionInfo failed: %d",GetLastError());
		return 0;
	}
	LPDWORD lpdwLangCp;

	if (!VerQueryValue(lpBuffer, TEXT("\\VarFileInfo\\Translation"), 
		(LPVOID*) &lpdwLangCp, &dwUint) ) 
	{
		printf("VerQueryValue failed:%d ", GetLastError());
		return 0;
	}
	TCHAR     szLangCp[9];
	wsprintf( szLangCp, TEXT ("%08X"),SWAPWORDS( *lpdwLangCp ));
	TCHAR SubBlock [2048];
	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%s\\FileVersion"), szLangCp );

	if (!VerQueryValue(lpBuffer, SubBlock,  (LPVOID*)&Buffer, &dwUint) ) 
	{
		printf("No file version info available");
		return 0;
	}
	else
		printf("File Version: %s", Buffer);
	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%s\\ProductVersion"), 
		szLangCp );
	if (!VerQueryValue(lpBuffer, SubBlock,  (LPVOID*)&Buffer, &dwUint) ) 
	{	
		printf("No produce version info available");
		return 0;
	}
	else
	{

		printf( "Product Version: %s", Buffer);
	}

	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%s\\FileDescription"), 
		szLangCp );
	if (!VerQueryValue(lpBuffer, SubBlock,  (LPVOID*)&Buffer, &dwUint) ) 
	{
		printf("No File Description info available");
		return 0;
	}
	else
	{
		printf("File Description: %s", Buffer);

			int len = _tcslen(Buffer);
		//	_tcsncpy(ProcessList[index].info, szModuleName,len);
			

	//	_tcscpy(buf, Buffer);
		_tcsncpy(buf, Buffer,len);
		buf[len] ='\0';

	}

	delete [] lpBuffer;

	return 1;
}


