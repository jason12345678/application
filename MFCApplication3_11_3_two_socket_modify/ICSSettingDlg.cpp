// ICSSettingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ICSSettingDlg.h"
#include "afxdialogex.h"

#include "stdafx.h"
#include "SmartSharingApp.h"
#include "SmartSharingDlg.h"

#include <NetCon.h>  

INetSharingConfiguration *pNSC[5] = {NULL};
INetSharingConfiguration *pVWifiNSC = NULL;
// CICSSettingDlg 对话框

IMPLEMENT_DYNAMIC(CICSSettingDlg, CDialogEx)

CICSSettingDlg::CICSSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CICSSettingDlg::IDD, pParent)
{
}

BOOL CICSSettingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	ICSConfigure();
	//m_cbInterface.AddString(_T("wqj;"));
	//m_cbInterface.AddString(_T("wqj;"));
	 
	if(m_cbInterface.GetCount()==0)
	{
		WINDOWPLACEMENT wp;
		wp.length=sizeof(WINDOWPLACEMENT);
		wp.flags=WPF_RESTORETOMAXIMIZED;
		wp.showCmd=SW_HIDE;
		SetWindowPlacement(&wp);
		AfxMessageBox(_T("没有可用的网络"));
		//this->CloseWindow();

		return FALSE;
	
	}
	m_cbInterface.SetCurSel(0);

	return TRUE;
}

CICSSettingDlg::~CICSSettingDlg()
{
}

void CICSSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ALL_CONNECT_INTERFACE, m_cbInterface);
}


BEGIN_MESSAGE_MAP(CICSSettingDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CICSSettingDlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_ALL_CONNECT_INTERFACE, &CICSSettingDlg::OnCbnSelchangeAllConnectInterface)
END_MESSAGE_MAP()


// CICSSettingDlg 消息处理程序

void CICSSettingDlg::ICSConfigure()
{
	// return;
	INetConnection * pNC = NULL; // fill this out for part 2 below
	INetSharingEveryConnectionCollection * pNSECC = NULL;
    INetSharingManager * pNSM = NULL;  
	INetSharingConfiguration* pNSCLocal= NULL;
	HRESULT hr = 0;
	CString s;
	CComPtr<INetSharingManager> netSharingMgr = NULL;

	hr = CoInitialize(NULL);
	if (!SUCCEEDED(hr))
	{
		s.Format(_T("CoInitialize error."));
		//AfxMessageBox(s);	
	}
	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (!SUCCEEDED(hr))
	{
		s.Format(_T("CoInitializeSecurity error."));
		//AfxMessageBox(s);	
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
		s.Format(_T("get_EnumEveryConnection error:%x."), hr);
		AfxMessageBox(s);	
	}
	else
	{
		//s.Format(_T("get_EnumEveryConnection success:%x."), hr);
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
			NETCON_PROPERTIES* pVWifi = NULL;
			int index = 0;

            while (S_OK == pEV->Next (1, &v, NULL)) 
			{
                if (V_VT (&v) == VT_UNKNOWN) 
				{
                    V_UNKNOWN (&v)->QueryInterface (__uuidof(INetConnection), (void**)&pNC);
                    if (pNC) 
					{
						NETCON_PROPERTIES* pNP = NULL;
						pNC->GetProperties(&pNP);
						hr = pNSM->get_INetSharingConfigurationForINetConnection(pNC, &pNSCLocal);
						
						setlocale(LC_ALL, "chs");
						char strGuid[GUID_STR_LEN];
						if(pNP->Status == NCS_CONNECTED)
						{
							if(CWfdWlanSolution::Instance()->getVirtualAdapterGUID(strGuid)==WFD_ERROR_SUCCESS)
							{
								if (!CWfdWlanSolution::Instance()->IsSameGUID(strGuid,&(pNP->guidId)))
								{
									m_cbInterface.AddString(pNP->pszwDeviceName);
									hr = pNSM->get_INetSharingConfigurationForINetConnection(pNC, &pNSC[index]);
									pNSC[index]->DisableSharing();
									index++;
								}
							}
						}
						//VARIANT_BOOL pbEnable=false;
						//pNSCLocal->get_SharingEnabled(&pbEnable);
						//if(pbEnable)
						//	pNSCLocal->DisableSharing();

						if(CWfdWlanSolution::Instance()->getVirtualAdapterGUID(strGuid)==WFD_ERROR_SUCCESS)
						{
							if (CWfdWlanSolution::Instance()->IsSameGUID(strGuid,&(pNP->guidId)))
							{
								pVWifi = pNP;
								hr = pNSM->get_INetSharingConfigurationForINetConnection(pNC,&pVWifiNSC);
								pVWifiNSC->DisableSharing();
							}
						}
					}
				}
			}
		}
	}
}

void CICSSettingDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	HRESULT hr;
	int index = m_cbInterface.GetCurSel();
	CString s;

	if ((!pNSC[index]) || (!pVWifiNSC))
	{
		AfxMessageBox(_T("set ics failed"));
		CDialogEx::OnOK();
		return;
	}
	hr = pNSC[index]->EnableSharing(ICSSHARINGTYPE_PUBLIC);
	if (S_OK != hr)
	{
		AfxMessageBox(_T("set public failed"));
	}
	hr = pVWifiNSC->EnableSharing(ICSSHARINGTYPE_PRIVATE);
	if (S_OK != hr)
	{//E_ABORT
		s.Format(_T("set private error:%d"), hr);
		AfxMessageBox(s);
	}
	AfxMessageBox(_T("set ics success"));
	CDialogEx::OnOK();
}


void CICSSettingDlg::OnCbnSelchangeAllConnectInterface()
{
	// TODO: 在此添加控件通知处理程序代码
}
