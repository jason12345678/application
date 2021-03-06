
// MFCApplication3.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "SmartSharingApp.h"
#include "SmartSharingDlg.h"
#include "StartSrvDlg.h"
#include "SmartSharingDlg1.h"
#include "log.h"

#include <NetCon.h>  
#pragma comment(lib,"Iphlpapi.lib") 
#pragma comment(lib,"Rpcrt4.lib")//
#pragma comment(lib,"ole32.lib")  

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SmartSharingDlg *mainDlg;
extern int g_bDebug;
// SmartSharingApp

BEGIN_MESSAGE_MAP(SmartSharingApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


HANDLE hPeerChangeEvent;
CRITICAL_SECTION m_PeerChangeCond;
CString rsnMac;
WLANEVTTYPE wlanEvtType;
CRITICAL_SECTION m_clientChangeCond;
ClientInfo clients[CLIENT_NUM] = {0};
// SmartSharingApp 构造

SmartSharingApp::SmartSharingApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
        set_key();
	if(get_key()){

		g_bDebug = 1;
	}
	//trace("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

SmartSharingApp::~SmartSharingApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	CString cmd;
	stopICS();
	StopSoftAP();
	CWfdWlanSolution::Instance()->DisConnectToAP();
	cmd.Format(_T("interface ip set address name=%s dhcp"), wlanName);
	ShellExecute(NULL, L"open", L"netsh.exe", (LPWSTR)(LPCWSTR)cmd, L"", SW_HIDE);
	ShellExecute(NULL, L"open", L"ipconfig.exe", _T("/release"), L"", SW_HIDE);
	ShellExecute(NULL, L"open", L"ipconfig.exe", _T("/renew"), L"", SW_HIDE);

	cmd.Format(_T("interface ip set address name=%s dhcp"), virtualName);
	ShellExecute(NULL, L"open", L"netsh.exe", (LPWSTR)(LPCWSTR)cmd, L"", SW_HIDE);
	ShellExecute(NULL, L"open", L"ipconfig.exe", _T("/release"), L"", SW_HIDE);
	ShellExecute(NULL, L"open", L"ipconfig.exe", _T("/renew"), L"", SW_HIDE);
}
// 唯一的一个 SmartSharingApp 对象

SmartSharingApp theApp;


// SmartSharingApp 初始化

BOOL SmartSharingApp::InitInstance()
{
	AfxOleInit();
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	HWND hPreWnd;

    if (hPreWnd = ::FindWindow(NULL, _T("SmartSharing")))
    {
        if (IsIconic(hPreWnd))  // IsIconic: Is Minisized? IsZoomed: Is Maxisized?
         {
            ::ShowWindow( hPreWnd, SW_SHOWNORMAL );
            SetForegroundWindow(hPreWnd);
        }
        else
        {
            ::ShowWindow( hPreWnd, SW_RESTORE );
            SetForegroundWindow(hPreWnd);
        }

        return TRUE;
    }

	CWinApp::InitInstance();
	InitializeCriticalSection(&m_PeerChangeCond);
	InitializeCriticalSection(&m_clientChangeCond);
	hPeerChangeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	wlanEvtType = WLAN_EVT_INVAILD;
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
	// CStartSrvDlg dlgSrv(NULL, &fileFlag, &icsFlag, &dsFlag);
	// dlgSrv.DoModal();
	StartSoftAP();
	//setSoftApAddr();
	//stopICS();
	// startICS();
	CString m_wlanIP;
	CString m_SoftApIP;
	CString path;
	::GetCurrentDirectory(2000,path.GetBuffer(2000));
	path.ReleaseBuffer();
	path +="\\launch.ini";
	path.Replace('\\', '/');
	::GetPrivateProfileStringW(_T("Setting"),_T("wlanIP"),_T("192.168.137.102"),m_wlanIP.GetBuffer(MAX_PATH),MAX_PATH, path);  
	::GetPrivateProfileStringW(_T("Setting"),_T("SoftApIP"),_T("192.168.137.1"),m_SoftApIP.GetBuffer(MAX_PATH),MAX_PATH, path);
	m_wlanIP.ReleaseBuffer();
	m_SoftApIP.ReleaseBuffer();
	if (false == setSoftApAddr(_T("NULL"), m_SoftApIP))
	{
		StopSoftAP();
		return FALSE;
	}
	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
	mainDlg = new  SmartSharingDlg();//SmartSharingDlg();;
	m_pMainWnd = mainDlg;
	INT_PTR nResponse = mainDlg->DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		//TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		//TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

CString strSSID;
BYTE macAddr[6] = {0};
DWORD SmartSharingApp::StartSoftAP(BOOL fileTransFlag, BOOL icsFlag, BOOL sharDskFlag)
{  
    // Need to assign
    CString strSecondaryKey;
    in_addr ip_local_address;

    //strSSID = pWnd->m_pSsidEdit->GetText();
    //strSecondaryKey = pWnd->m_pKeyEdit->GetText();
    strSecondaryKey = _T("11111111");
	if (strcmp((char*)macAddr,"\0\0\0\0\0")==0)
	{
		CWfdWlanSolution::Instance()->getWifiCardMacAddr(macAddr);
	}
	if(macAddr==NULL)
    {
        AfxMessageBox(_T("There is no Wi-Fi card on this machine, this application needs one to run\n"));
        exit(0);
    }
	/*
	
	    DWORD dwNameLen = 512; 
		TCHAR  szBuffer[512];  
		memset(szBuffer,0,sizeof(szBuffer));
    if (!GetComputerNameW(szBuffer, &dwNameLen))   
        printf("Error  %d\n", GetLastError());  
    else  
        printf("计算机名为: %s\n", szBuffer);  
		*/

	strSSID.Format(_T("SmartSharing-%02x%02x%02x%02x%02x%02x-%d-%d-%d"), macAddr[0], macAddr[1], macAddr[2],
		macAddr[3], macAddr[4], macAddr[5],fileTransFlag, icsFlag, sharDskFlag);

    int nRet = CWfdWlanSolution::Instance()->HostedNetworkStartSoftAp(strSSID, strSecondaryKey, ip_local_address);

    switch (nRet)
    {
    case WFD_ERROR_HOSTEDNETWORK_START_FAILED:
        MessageBox(NULL, _T("WFD_ERROR_HOSTEDNETWORK_START_FAILED."), _T("Error"), MB_OK);
        break;
    case WFD_ERROR_HOSTEDNETWORK_INVALID_STATUS:
        MessageBox(NULL, _T("WFD_ERROR_HOSTEDNETWORK_INVALID_STATUS."), _T("Error"), MB_OK);
        break;
    case WFD_ERROR_HOSTEDNETWORK_ALLOW_FAILED:
        MessageBox(NULL, _T("WFD_ERROR_HOSTEDNETWORK_ALLOW_FAILED."), _T("Error"), MB_OK);
        break;
    case WFD_ERROR_HOSTEDNETWORK_NIC_NOT_ENABLE:
				
        MessageBox(NULL, _T("WFD_ERROR_HOSTEDNETWORK_NIC_NOT_ENABLE."), _T("Error"), MB_OK);
        break;
    default:
        //MessageBox(NULL, _T("WFD_ERROR_SUCCESS."), _T("Error"), MB_OK);
        break;
    }

    // update to ui code

    return 0;
}

DWORD SmartSharingApp::StopSoftAP()
{
	/*
	system("netsh wlan stop hostednetwork");
	return 0;
	*/
    int nRet = CWfdWlanSolution::Instance()->HostedNetworkStopSoftAp();

    switch (nRet)
    {
    case WFD_ERROR_HOSTEDNETWORK_STOP_FAILED:
        MessageBox(NULL, _T("WFD_ERROR_HOSTEDNETWORK_STOP_FAILED."), _T("Error"), MB_OK);
        break;
    default:
        //MessageBox(NULL, _T("WFD_ERROR_SUCCESS."), _T("Error"), MB_OK);
        break;
    }

    
    return 0;
}

DWORD SmartSharingApp::ScanAllBssInfo(_In_  LPVOID lpParameter)
{
    SmartSharingDlg * pWnd = (SmartSharingDlg *)lpParameter;   
    std::vector<WLAN_BSS_ENTRY> vecSelectedBSSEntry;
    vecSelectedBSSEntry.clear();
    int nRet = CWfdWlanSolution::Instance()->ScanBssList(vecSelectedBSSEntry, true);

    switch (nRet)
    {
    case WFD_ERROR_WLAN_NO_VALID_INTERFACE:
        MessageBox(NULL, _T("No wireless card worked, please check your machine."), _T("Error"), MB_OK);
        break;
    case WFD_ERROR_WLAN_SCAN_FAILED:
        MessageBox(NULL, _T("WFD_ERROR_WLAN_SCAN_FAILED."), _T("Error"), MB_OK);
        break;
    case WFD_ERROR_WLAN_SCAN_TIMEOUT:
        MessageBox(NULL, _T("WFD_ERROR_WLAN_SCAN_TIMEOUT."), _T("Error"), MB_OK);
        break;
    case WFD_ERROR_WLAN_GET_BSS_LIST_FAILED:
        MessageBox(NULL, _T("WFD_ERROR_WLAN_GET_BSS_LIST_FAILED."), _T("Error"), MB_OK);
        break;
    default:
        MessageBox(NULL, _T("WFD_ERROR_SUCCESS."), _T("Error"), MB_OK);
        break;
    }
	for (int i=0; i<vecSelectedBSSEntry.size(); i++)
	{
		CString s(vecSelectedBSSEntry.at(i).dot11Ssid.ucSSID);
		MessageBox(NULL, s, _T("Error"), MB_OK);
	}

    // add text to ui control

    return 0;
}

 DWORD SmartSharingApp::ConnectToAP(_In_  LPVOID lpParameter)
{
    // CWifiDisplaySourceWnd * pWnd = (CWifiDisplaySourceWnd *)lpParameter;  

    // Need to assign
    unsigned int nConnectBssEntryIndex = 0;
    CString strSecondaryKey(_T("12345678"));
    in_addr ip_local_address;
    in_addr ip_gateway_address;

    int nRet = CWfdWlanSolution::Instance()->ConnectToAP(nConnectBssEntryIndex, strSecondaryKey, ip_local_address, ip_gateway_address);

    switch (nRet)
    {
    case WFD_ERROR_WLAN_CONNECT_TIMEOUT:
        MessageBox(NULL, _T("WFD_ERROR_WLAN_CONNECT_TIMEOUT."), _T("Error"), MB_OK);
        break;
    case WFD_ERROR_WLAN_CONNECT_FAILED:
        MessageBox(NULL, _T("WFD_ERROR_WLAN_CONNECT_FAILED."), _T("Error"), MB_OK);
        break;
    case WFD_ERROR_WLAN_GET_IP_FAILED:
        MessageBox(NULL, _T("WFD_ERROR_WLAN_GET_IP_FAILED."), _T("Error"), MB_OK);
        break;
    default:
        MessageBox(NULL, _T("WFD_ERROR_SUCCESS."), _T("Error"), MB_OK);
        break;
    }    

    // update ui code here


    return 0;
}

 void SmartSharingApp::stopICS()
 {
 	INetConnection * pNC = NULL; // fill this out for part 2 below
	INetSharingEveryConnectionCollection * pNSECC = NULL;
    INetSharingManager * pNSM = NULL;  
	HRESULT hr = 0;
	CString s;
	CComPtr<INetSharingManager> netSharingMgr = NULL;
	INetSharingConfiguration * pNSC = NULL;
	INetSharingConfiguration * pVWifiNSC = NULL;

	hr = CoInitialize(NULL);
	if (!SUCCEEDED(hr))
	{
		//s.Format(_T("CoInitialize error."));
		//AfxMessageBox(s);	
	}
	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (!SUCCEEDED(hr))
	{
		//s.Format(_T("CoInitializeSecurity error."));
		//AfxMessageBox(s);	
	}
	//hr = netSharingMgr.CoCreateInstance(CLSID_NetSharingManager);
    hr = ::CoCreateInstance(__uuidof(NetSharingManager), NULL, CLSCTX_ALL, __uuidof(INetSharingManager), (void**)&pNSM);
	if (S_OK != hr)
	{
		s.Format(_T("CoCreateInstance error."));
		//AfxMessageBox(s);	
	}
	//hr = netSharingMgr->get_EnumEveryConnection(&pNSECC);
	hr = pNSM->get_EnumEveryConnection(&pNSECC);
	if (!SUCCEEDED(hr))
	{// E_FAIL
		s.Format(_T("get_EnumEveryConnection error:%x."), hr);
		//AfxMessageBox(s);	
	}
	else
	{
		s.Format(_T("get_EnumEveryConnection success:%x."), hr);
		//AfxMessageBox(s);	
	}
	
    if (!pNSECC)
	{
        wprintf (L"failed to get EveryConnectionCollection!\r\n");
	}
    else 
	{
        // enumerate connections
        IEnumVARIANT * pEV = NULL;
        IUnknown * pUnk = NULL;

        hr = pNSECC->get__NewEnum (&pUnk);

        if (pUnk)
		{
            hr = pUnk->QueryInterface (__uuidof(IEnumVARIANT), (void**)&pEV);
            pUnk->Release();

        }
		if (pEV) 
		{
            VARIANT v;
            VariantInit(&v);
            BOOL bFoundIt = FALSE;

            while (S_OK == pEV->Next (1, &v, NULL)) 
			{
                if (V_VT (&v) == VT_UNKNOWN) 
				{
                    V_UNKNOWN (&v)->QueryInterface (__uuidof(INetConnection), (void**)&pNC);
                    if (pNC) 
					{
						NETCON_PROPERTIES* pNP = NULL;
						pNC->GetProperties(&pNP);
						setlocale(LC_ALL, "chs");
						hr = pNSM->get_INetSharingConfigurationForINetConnection(pNC, &pNSC);
						pNSC->DisableSharing();
					}
				}
			}
		}
	}
 }


void SmartSharingApp::startICS()
{
	// return;
	INetConnection * pNC = NULL; // fill this out for part 2 below
	INetSharingEveryConnectionCollection * pNSECC = NULL;
    INetSharingManager * pNSM = NULL;  
	HRESULT hr = 0;
	CString s;
	CComPtr<INetSharingManager> netSharingMgr = NULL;

	hr = CoInitialize(NULL);
	if (!SUCCEEDED(hr))
	{
		s.Format(_T("CoInitialize error. Please run with elevated privileges!"));
		//AfxMessageBox(s);	
	}
	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (!SUCCEEDED(hr))
	{
		s.Format(_T("CoInitializeSecurity error.Please run with elevated privileges!"));
		//AfxMessageBox(s);	
	}
	//hr = netSharingMgr.CoCreateInstance(CLSID_NetSharingManager);
    hr = ::CoCreateInstance(__uuidof(NetSharingManager), NULL, CLSCTX_ALL, __uuidof(INetSharingManager), (void**)&pNSM);
	if (S_OK != hr)
	{
		s.Format(_T("CoCreateInstance error.Please run with elevated privileges!"));
		AfxMessageBox(s);	
	}
	//hr = netSharingMgr->get_EnumEveryConnection(&pNSECC);
	hr = pNSM->get_EnumEveryConnection(&pNSECC);
	if (!SUCCEEDED(hr))
	{// E_FAIL
		s.Format(_T("get_EnumEveryConnection error:%x.Please run with elevated privileges!"), hr);
		AfxMessageBox(s);	
	}
	else
	{
		s.Format(_T("get_EnumEveryConnection success:%x."), hr);
		AfxMessageBox(s);	
	}
	
    if (!pNSECC)
	{
        wprintf (L"failed to get EveryConnectionCollection!\r\n");
	}
    else 
	{
        // enumerate connections
        IEnumVARIANT * pEV = NULL;
        IUnknown * pUnk = NULL;

        hr = pNSECC->get__NewEnum (&pUnk);

        if (pUnk)
		{
            hr = pUnk->QueryInterface (__uuidof(IEnumVARIANT), (void**)&pEV);
            pUnk->Release();

        }
		if (pEV) 
		{
            VARIANT v;
            VariantInit(&v);
            BOOL bFoundIt = FALSE;
			INetSharingConfiguration * pNSC = NULL;
			INetSharingConfiguration * pVWifiNSC = NULL;
			NETCON_PROPERTIES* pVWifi = NULL;

            while (S_OK == pEV->Next (1, &v, NULL)) 
			{
                if (V_VT (&v) == VT_UNKNOWN) 
				{
                    V_UNKNOWN (&v)->QueryInterface (__uuidof(INetConnection), (void**)&pNC);
                    if (pNC) 
					{
						NETCON_PROPERTIES* pNP = NULL;
						pNC->GetProperties(&pNP);
						setlocale(LC_ALL, "chs");
						CString s;
						// if(pNP->Status == NCS_CONNECTED)
						/*if (!strcmp((char*)(pNP->pszwName),(char*)(L"Wi-Fi")))
						{
							hr = pNSM->get_INetSharingConfigurationForINetConnection (pNC, &pNSC);
							hr = pNSC->EnableSharing(ICSSHARINGTYPE_PUBLIC);
							if (S_OK == hr)
							{//E_ABORT
								// AfxMessageBox(_T("set public success"));
							}
							//pNSC->Release();
						}*/
						char strGuid[GUID_STR_LEN];
						if(CWfdWlanSolution::Instance()->getWlanAdapterGUID(strGuid)==WFD_ERROR_SUCCESS)
						{
							if (CWfdWlanSolution::Instance()->IsSameGUID(strGuid,&(pNP->guidId)))
							{
								hr = pNSM->get_INetSharingConfigurationForINetConnection (pNC, &pNSC);
								hr = pNSC->EnableSharing(ICSSHARINGTYPE_PUBLIC);
								if (S_OK == hr)
								{
									//AfxMessageBox(_T("set public success"));
								}
								//pNSC->Release();
							}
						}	
						if(CWfdWlanSolution::Instance()->getVirtualAdapterGUID(strGuid)==WFD_ERROR_SUCCESS)
						{
							if (CWfdWlanSolution::Instance()->IsSameGUID(strGuid,&(pNP->guidId)))
							{
								pVWifi = pNP;
								hr = pNSM->get_INetSharingConfigurationForINetConnection(pNC,&pVWifiNSC);
								pVWifiNSC->EnableSharing(ICSSHARINGTYPE_PRIVATE);
								if (S_OK == hr)
								{
									//AfxMessageBox(_T("set private success"));
								}
								else
								{
									//AfxMessageBox(_T("set private failed"));
								}
							}
						}
					}
				}
			}
		}
	}
}

bool SmartSharingApp::setSoftApAddr(CString wlanIP, CString softApIP)
{
	INetConnection * pNC = NULL; // fill this out for part 2 below
	INetSharingEveryConnectionCollection * pNSECC = NULL;
    INetSharingManager * pNSM = NULL;  
	HRESULT hr = 0;
	CString s;
	CComPtr<INetSharingManager> netSharingMgr = NULL;

	hr = CoInitialize(NULL);
	if (!SUCCEEDED(hr))
	{
		s.Format(_T("CoInitialize error."));
		AfxMessageBox(s);	
	}
	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (!SUCCEEDED(hr))
	{
		s.Format(_T("CoInitializeSecurity error."));
		AfxMessageBox(s);	
	}
	//hr = netSharingMgr.CoCreateInstance(CLSID_NetSharingManager);
    hr = ::CoCreateInstance(__uuidof(NetSharingManager), NULL, CLSCTX_ALL, __uuidof(INetSharingManager), (void**)&pNSM);
	if (S_OK != hr)
	{
		s.Format(_T("CoCreateInstance error."));
		AfxMessageBox(s);	
	}
	//hr = netSharingMgr->get_EnumEveryConnection(&pNSECC);
	hr = pNSM->get_EnumEveryConnection(&pNSECC);
	if (!SUCCEEDED(hr))
	{// E_FAIL
		s.Format(_T("请用管理员权限启动该应用！"));
		AfxMessageBox(s);	
		return false;
	}
	else
	{
		s.Format(_T("get_EnumEveryConnection success:%x."), hr);
		// AfxMessageBox(s);	
	}
	
    if (!pNSECC)
	{
        wprintf (L"failed to get EveryConnectionCollection!\r\n");
	}
    else 
	{
        // enumerate connections
        IEnumVARIANT * pEV = NULL;
        IUnknown * pUnk = NULL;

        hr = pNSECC->get__NewEnum (&pUnk);

        if (pUnk)
		{
            hr = pUnk->QueryInterface (__uuidof(IEnumVARIANT), (void**)&pEV);
            pUnk->Release();

        }
		if (pEV) 
		{
            VARIANT v;
            VariantInit(&v);
            BOOL bFoundIt = FALSE;
			INetSharingConfiguration * pNSC = NULL;
			INetSharingConfiguration * pVWifiNSC = NULL;
			NETCON_PROPERTIES* pVWifi = NULL;

            while (S_OK == pEV->Next (1, &v, NULL)) 
			{
                if (V_VT (&v) == VT_UNKNOWN) 
				{
                    V_UNKNOWN (&v)->QueryInterface (__uuidof(INetConnection), (void**)&pNC);
                    if (pNC) 
					{
						NETCON_PROPERTIES* pNP = NULL;
						pNC->GetProperties(&pNP);
						setlocale(LC_ALL, "chs");
						CString cmd;
						char strGuid[GUID_STR_LEN];
						if(CWfdWlanSolution::Instance()->getWlanAdapterGUID(strGuid)==WFD_ERROR_SUCCESS)
						{
							if (CWfdWlanSolution::Instance()->IsSameGUID(strGuid,&(pNP->guidId)))
							{
								if (wlanIP != _T("NULL"))
								{
									wlanName.Format(_T("\"%s\""), pNP->pszwName);
									cmd.Format(_T("interface ip set address name=%s static "+wlanIP+" 255.255.255.0"), wlanName);
									HINSTANCE rst = ShellExecute(NULL, L"open", L"netsh.exe", (LPWSTR)(LPCWSTR)cmd, L"", SW_HIDE);
								}
							}
						}
						if(CWfdWlanSolution::Instance()->getVirtualAdapterGUID(strGuid)==WFD_ERROR_SUCCESS)
						{
							if (CWfdWlanSolution::Instance()->IsSameGUID(strGuid,&(pNP->guidId)))
							{
								if (softApIP != _T("NULL"))
								{
									virtualName.Format(_T("\"%s\""), pNP->pszwName);
									cmd.Format(_T("interface ip set address name=%s static "+softApIP+" 255.255.255.0"), virtualName);
									HINSTANCE rst = ShellExecute(NULL, L"open", L"netsh.exe", (LPWSTR)(LPCWSTR)cmd, L"", SW_HIDE);
								}
							}
						}
					}
				}
			}
		}
	}
	return true;
}
