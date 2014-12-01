#include "stdafx.h"
#include "SmartSharingApp.h"
#include "SmartSharingDlg.h"
CLocalState::CLocalState(void)
{
    LocalState = DISCONNECT;
}

CLocalState::CLocalState(void * pParentDlg)
{
	m_pParentDialog = pParentDlg;
	LocalState = DISCONNECT;
}

CLocalState::~CLocalState(void)
{
	m_pParentDialog=NULL;
}

void CLocalState::GoToConnected(CString SSID)
{
    ConnectedPeerSSID=SSID;
	if(LocalState ==  CONNECTED)
		return;
    LocalState = CONNECTED;
	
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_LIST_CONN)->EnableWindow(false);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_RESTART)->EnableWindow(false);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_BUTTON3)->EnableWindow(false);

	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_ICS_CHKBOX)->EnableWindow(false);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_FILE_CHKBOX)->EnableWindow(false);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_DS_CHKBOX)->EnableWindow(false);



	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_LIST_CONT)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_BTNSCAN)->EnableWindow(true);
	
	

	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_STATIC_STATUS)->SetWindowTextW(_T("I have connected to other device, cannot start service until disconnection."));

	//To do: should start ICS only in "service running" state, no mater if "enable ICS" checked(the purpose is to make sure virtual adapter get a valid ip)
	theApp.stopICS();

	(((SmartSharingDlg *)m_pParentDialog)->m_fileSrvFlag).SetCheck(0);
	(((SmartSharingDlg *)m_pParentDialog)->m_ICSSrvFlag).SetCheck(0);
	(((SmartSharingDlg *)m_pParentDialog)->m_DSSrvFlag).SetCheck(0);


}

void CLocalState::GotoNormal()
{

	LocalState = NORMAL;
	 ConnectedPeerSSID = "";
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_LIST_CONN)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_RESTART)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_ICS_CHKBOX)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_FILE_CHKBOX)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_DS_CHKBOX)->EnableWindow(true);

	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_LIST_CONT)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_BTNSCAN)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(DC_STOP_SRV)->EnableWindow(false);

	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_STATIC_STATUS)->SetWindowTextW(_T("I have no services started, others could see but can not connect to me."));

	ShellExecute(NULL, L"open", L"netsh.exe", L"wlan set hostednetwork mode=allow", L"", SW_HIDE);
	Sleep(3000);
	theApp.StartSoftAP(theApp.fileFlag, theApp.icsFlag, theApp.dsFlag);


	
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_BUTTON3)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_BUTTON4)->EnableWindow(false);
	int i;
	for(i=IDC_BUTTON6;i<=IDC_BUTTON15;i++)
	{
		((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(i)->EnableWindow(true);
		
	}
}

void CLocalState::GotoDisconnect()
{
	CString ssid;
	int itemCnt = 0;
	CString itemSSID;
	if(LocalState ==  DISCONNECT)
		return;
	LocalState = DISCONNECT;
	 ConnectedPeerSSID = "";
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_LIST_CONN)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_RESTART)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_ICS_CHKBOX)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_FILE_CHKBOX)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_DS_CHKBOX)->EnableWindow(true);

	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_LIST_CONT)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_BTNSCAN)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(DC_STOP_SRV)->EnableWindow(false);

	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_STATIC_STATUS)->SetWindowTextW(_T("I have no services started, others could see but can not connect to me."));

	ssid = ((SmartSharingDlg *)m_pParentDialog)->m_LocalState.GetConnectedSSID();
	itemCnt = ((SmartSharingDlg *)m_pParentDialog)->m_listCont.GetItemCount();
	for (int i=0; i<itemCnt; i++)
	{
		itemSSID = ((SmartSharingDlg *)m_pParentDialog)->m_listCont.GetItemText(i, 0);
		if (itemSSID == ssid)
		{
			((SmartSharingDlg *)m_pParentDialog)->m_listCont.SetItemText(i, 1, _T("Disconnected"));
		}
	}
	ShellExecute(NULL, L"open", L"netsh.exe", L"wlan set hostednetwork mode=allow", L"", SW_HIDE);
	Sleep(3000);
	theApp.StartSoftAP(theApp.fileFlag, theApp.icsFlag, theApp.dsFlag);


	
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_BUTTON3)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_BUTTON4)->EnableWindow(false);
	int i;
	for(i=IDC_BUTTON6;i<=IDC_BUTTON15;i++)
	{
		((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(i)->EnableWindow(true);
		
	}

}

void CLocalState::GotoSendFileWaitAck()
{
	LocalState = SENDFILEWAITACK;
}
void CLocalState::GotoSendFileAckCancel()
{
	LocalState = CONNECTED;
}

void CLocalState::GotoSendFileAckOK()
{
	LocalState = SENDFILEACKOK;
}

void CLocalState::GotoSendFileAckEnd()
{
	LocalState = CONNECTED;
}

void CLocalState::GotoServiceRun()
{
	if(LocalState ==  SERVICERUN)
		return;
	LocalState = SERVICERUN;
	
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_LIST_CONN)->EnableWindow(true);
	
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_RESTART)->EnableWindow(false);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_ICS_CHKBOX)->EnableWindow(false);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_FILE_CHKBOX)->EnableWindow(false);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_DS_CHKBOX)->EnableWindow(false);
	
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_LIST_CONT)->EnableWindow(false);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_BTNSCAN)->EnableWindow(true);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(DC_STOP_SRV)->EnableWindow(true);

	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_STATIC_STATUS)->SetWindowTextW(_T("I'm servicing others, will not connect to anyone until services stop."));


	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_BUTTON3)->EnableWindow(false);
	((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(IDC_BUTTON4)->EnableWindow(true);
	int i;
	for(i=IDC_BUTTON6;i<=IDC_BUTTON15;i++)
	{
		((SmartSharingDlg *)m_pParentDialog)->GetDlgItem(i)->EnableWindow(false);
		
	}


}

CLocalState::STATE CLocalState::GetCurrentState()
{
    return LocalState;
}


CString CLocalState::GetConnectedSSID()
{
    return ConnectedPeerSSID;
}

void CLocalState::SetParent(LPVOID pParentDlg)
{
	m_pParentDialog = pParentDlg;
}