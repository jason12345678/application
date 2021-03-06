// asav_sampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "asav_sample.h"
#include "asav_sampleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int PRIO_LVE = 0;
int PRE_LVE = 0;
// CAboutDlg dialog used for App About

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
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_ADD, &Casav_sampleDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, &Casav_sampleDlg::OnBnClickedBtnDel)
	ON_BN_CLICKED(IDC_BTN_UP, &Casav_sampleDlg::OnBnClickedBtnUp)
	ON_BN_CLICKED(IDC_BTN_DOWN, &Casav_sampleDlg::OnBnClickedBtnDown)
	ON_BN_CLICKED(IDC_BTN_APPLY, &Casav_sampleDlg::OnBnClickedBtnApply)
	ON_BN_CLICKED(IDC_BTN_SET, &Casav_sampleDlg::OnBnClickedBtnSet)
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
    GetWindowText(caption, _countof(caption));
    _tcscat_s(caption, _countof(caption), SDK_VERSION);
    SetWindowText(caption);
	
	// Add item in right list control
	RECT rect;
    GetWindowRect(&m_rect);
    m_tab.GetWindowRect(&rect);
    m_rect_s = m_rect;
    m_rect_s.bottom = rect.top;
    //OnBnClickedBtnDetail();

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
	m_list_bw.InsertColumn(LIST_PID, _T("PID"), LVCFMT_LEFT, 100);
    m_list_bw.InsertColumn(LIST_PROCESS, _T("Process"), LVCFMT_LEFT, 200);
    m_list_bw.InsertColumn(LIST_TX_BW, _T("Tx Bandwidth"), LVCFMT_LEFT, 150);
    m_list_bw.InsertColumn(LIST_RX_BW, _T("Rx Bandwidth"), LVCFMT_LEFT, 150);
	m_list_bw.InsertColumn(LIST_PRIO, _T("Priority"), LVCFMT_LEFT, 100);
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
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

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
	switch(nIDEvent)
	{
		case 1: 
			{
				m_smart_net->lock();
				UpdateBandwidth();
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
    TCHAR str1[32];
	CString prio;
 

	for(int i=0;i<count;i++)
	{
		_itot_s(process[i].pid, str1, 10);
		m_list_bw.InsertItem(i, str1);
		m_list_bw.SetItemText(i, LIST_PROCESS, process[i].proc_name);
		m_list_bw.SetItemText(i, LIST_TX_BW, _T("0"));
		m_list_bw.SetItemText(i, LIST_RX_BW, _T("0"));
		m_list_bw.SetItemText(i, LIST_PRIO, _T("None"));
	}
}

void Casav_sampleDlg::UpdateListBw(PROCESS& process, int i)
{
    TCHAR str1[32], str2[32];
	CString prio;
    int j;
    DWORD pid;

    if (i < 0) {
        for (j=0; j<m_list_bw.GetItemCount(); j++) {
            m_list_bw.GetItemText(j, LIST_PID, str1, _countof(str1));
            pid = _ttoi(str1);
            if (pid == process.pid) {
                i = j;
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
	if(process.rxpi
	m_list_bw.SetItemText(i, LIST_PRIO, _T("Normal"));
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
	int i;
    POSITION pos;
    PROCESS process;
    pos = m_list_right.GetFirstSelectedItemPosition();
    if (pos) {
        i = m_list_right.GetNextSelectedItem(pos);
        if (i > 0) {
            pos = m_process_right.FindIndex(i);
            process = m_process_right.GetAt(pos);
            m_process_right.RemoveAt(pos);
            m_list_right.DeleteItem(i);

            pos = m_process_right.FindIndex(i-1);
            m_process_right.InsertBefore(pos, process);
            InsertToListRight(i-1, process);
            m_list_right.SetItemState(i-1, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
        }
        m_list_right.SetFocus();
    }
}

void Casav_sampleDlg::OnBnClickedBtnDown()
{
	// TODO: Add your control notification handler code here
	int i;
    POSITION pos;
    PROCESS process;
    pos = m_list_right.GetFirstSelectedItemPosition();
    if (pos) {
        i = m_list_right.GetNextSelectedItem(pos);
        if (i < (m_list_right.GetItemCount() - 1)) {
            pos = m_process_right.FindIndex(i);
            process = m_process_right.GetAt(pos);
            m_process_right.RemoveAt(pos);
            m_list_right.DeleteItem(i);

            pos = m_process_right.FindIndex(i);
            m_process_right.InsertAfter(pos, process);
            InsertToListRight(i+1, process);
            m_list_right.SetItemState(i+1, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
        }
        m_list_right.SetFocus();
    }
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
	int i;
    POSITION pos;
    PROCESS process;
	CString  proc_name;

	m_smart_net->auto_detect_bw_enabled(TRUE);

    m_smart_net->clearall_prio();
    for (i=0; i<m_list_bw.GetItemCount() && i < MAX_PRIORITY_LEVEL; i++) {
		proc_name = m_list_bw.GetItemText(i, LIST_PROCESS);
		if (proc_name.GetLength() > 0)
		{
			m_smart_net->set_proc_prio(m_list_bw.GetItemText(i, LIST_PROCESS), i+1);
			 _itot_s(i+1, str, 10);
			//m_list_bw.SetItemText(i, LIST_PRIO, str);//We do not need to show the priority on the screen
		}
	}

    m_smart_net->apply_prio_list();
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

void Casav_sampleDlg::UpdateBandwidth()
{
    int i;
    TCHAR str[64];
    DWORD pid;
    UINT32 tx_bw, rx_bw;
    PROCESS process;

	m_smart_net->proc_seek_head();
	
	while (ERROR_SUCCESS == m_smart_net->get_next_proc(process.proc_name, 
        _countof(process.proc_name), &process.pid, &process.rx_prio, &process.tx_prio, 
        &process.rx_bw, &process.tx_bw))
    {
        UpdateListBw(process, -1);
    }

  /*  for (i=0; i<m_list_bw.GetItemCount(); i++) {
        m_list_bw.GetItemText(i, LIST_PID, str, _countof(str));
        pid = _ttoi(str);
        m_smart_net->get_bw_statis_by_pid(pid, &rx_bw, &tx_bw);
        process.pid = pid;
        process.tx_bw = tx_bw;
        process.rx_bw = rx_bw;
        UpdateListBw(process, i);
    }
	*/
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
int Casav_sampleDlg::EnumProcess(PROCESS * ProcessList, long MaxSize)
{
	HANDLE hProcess;
	int index=0;
	BOOL bProcessEnd=FALSE;
	PROCESSENTRY32 Info, *pInfo;
	DWORD err;



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
		bProcessEnd=Process32Next(hProcess, pInfo);   
		index++;
		if(index >= MaxSize) 
			break;
	}
	CloseHandle(hProcess);
	return index;
}
