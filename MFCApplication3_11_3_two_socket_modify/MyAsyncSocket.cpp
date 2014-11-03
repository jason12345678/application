// MyAsyncSocket.cpp : 实现文件
//

#include "stdafx.h"
#include "SmartSharingApp.h"
#include "MyAsyncSocket.h"
#include "SmartSharingDlg.h"
#include "SendFileTipDlg.h"
#include "md5_check.h"
#include "log.h"
#include <map>
using namespace std;

// CMyAsyncSocket

//extern unsigned int ackPkg = 0;
//extern int g_md5_is_true = 0;

map<CString ,client_data *  >myMap;

CMyAsyncSocket::CMyAsyncSocket()
{
	//AfxMessageBox(_T("create my sock"));
	//dstFile.Open(_T("c:\\123\\123-cpy.txt"),
	//					CFile::modeReadWrite | CFile::shareExclusive | CFile::typeBinary | CFile::modeCreate);

}

CMyAsyncSocket::~CMyAsyncSocket()
{
}

// CMyAsyncSocket 成员函数
void CMyAsyncSocket::OnReceive(int nErrorCode)
{
	BYTE revPkg[PKGSIZE] = {0};
	static int index = 0;
	int nRead;
	CString fileName;
	int rst;
	CString pszFileName;
	char tmp[FILE_NAME_LENGTH] = {0};
	char dstFilePath[FILE_NAME_LENGTH] = {0};
	//char *tmp = NULL;
	CString srcIP;
	UINT srcPort = 0;
	CString s;
	CFileDialog *dlgFile = NULL;
	char *pPos = NULL;
	CString suffix;
	CString defName;
	CSendFileTipDlg dlg;
	char tmpSSID[32] = {0};
	wchar_t wbuf[100] = {0};

	message msg;
	int number;

	memset(revPkg, 0, sizeof(revPkg));
	nRead = ReceiveFrom(revPkg, sizeof(revPkg), srcIP, srcPort);

	//client_data *pData = new client_data();
	if(myMap[srcIP]==NULL){

		client_data *pData = new client_data();
		memcpy_s(pData->data, sizeof(revPkg),revPkg,sizeof(revPkg));
		pData->mySize = 0;
		pData->index = index;
		myMap[srcIP] =pData;
		index++;

	}
	else{

		memcpy_s(myMap[srcIP]->data, sizeof(revPkg),revPkg,sizeof(revPkg));
	}


	switch (nRead)
	{
	case 0:
		// Close();
        break;
	case SOCKET_ERROR:
		if (GetLastError() != WSAEWOULDBLOCK) 
		{
			s.Format(_T("error num:%d"), GetLastError());
			AfxMessageBox (s);
			// Close();
        }
        break;
	default:
		
		switch (myMap[srcIP]->data[0])
		{
		case TRANS_FILE_START:
			trace("send file start msg");
			/* Has been disallow when connect to other ap
			if (srcIP == _T("192.168.137.1") || srcIP == _T("192.168.173.1"))
			{
				ShellExecute(NULL, L"open", L"netsh.exe", L"wlan set hostednetwork mode=disallow", L"", SW_HIDE);
				Sleep(1000);
			}
			*/
			rst = dlg.DoModal();
			if (2 == rst) // cancel
			{
				/* disallow until disconnect from other ap
				if (srcIP == _T("192.168.137.1") || srcIP == _T("192.168.173.1"))
				{
					ShellExecute(NULL, L"open", L"netsh.exe", L"wlan set hostednetwork mode=allow", L"", SW_HIDE);
					//ShellExecute(NULL, L"open", L"netsh.exe", L"wlan start hostednetwork", L"", SW_HIDE);
					theApp.StartSoftAP(theApp.fileFlag, theApp.icsFlag, theApp.dsFlag);
					Sleep(1000);
				}

				*/
				myMap[srcIP]->data[0] = ACK_TRANS_FILE_START;
				myMap[srcIP]->data[1] = '0';//refused
				SendTo(myMap[srcIP]->data, PKGCONTOFFSET, srcPort, srcIP);
				return;
			}
			pPos = strchr((char *)(&revPkg[PKGCONTOFFSET]), '.');
			suffix = pPos+1;
			*pPos = '\0';
			defName = &revPkg[PKGCONTOFFSET];
			dlgFile = new CFileDialog(FALSE, suffix, defName);
			if (IDOK == dlgFile->DoModal())
			{
				s = dlgFile->GetPathName(); 
			}
			else
			{
				/* disallow until disconnect from other ap
				if (srcIP == _T("192.168.137.1") || srcIP == _T("192.168.173.1"))
				{
					ShellExecute(NULL, L"open", L"netsh.exe", L"wlan set hostednetwork mode=allow", L"", SW_HIDE);
					//ShellExecute(NULL, L"open", L"netsh.exe", L"wlan start hostednetwork", L"", SW_HIDE);
					theApp.StartSoftAP(theApp.fileFlag, theApp.icsFlag, theApp.dsFlag);
					Sleep(1000);
				}
				*/
				return;
			}
			//tmp = (LPSTR)(LPCTSTR)s;
			memcpy_s(tmp, 500, (LPSTR)(LPCTSTR)s, 500);
			for (int i=0,j=0; i<FILE_NAME_LENGTH;i++, j+=2)
			{
				dstFilePath[i] = tmp[j];
			}
			rst = fopen_s(&myMap[srcIP]->f, dstFilePath, "wb+");
			switch(rst)
			{
			case 0:
				break;
			case EACCES :
				/* disallow until disconnect from other ap
				if (srcIP == _T("192.168.137.1") || srcIP == _T("192.168.173.1"))
				{
					ShellExecute(NULL, L"open", L"netsh.exe", L"wlan set hostednetwork mode=allow", L"", SW_HIDE);
					//ShellExecute(NULL, L"open", L"netsh.exe", L"wlan start hostednetwork", L"", SW_HIDE);
					theApp.StartSoftAP(theApp.fileFlag, theApp.icsFlag, theApp.dsFlag);
					Sleep(1000);
				}
				*/
				fclose(myMap[srcIP]->f);
				AfxMessageBox(_T("Permission denied."));
				return;
			default:
				/* disallow until disconnect from other ap
				if (srcIP == _T("192.168.137.1") || srcIP == _T("192.168.173.1"))
				{
					ShellExecute(NULL, L"open", L"netsh.exe", L"wlan set hostednetwork mode=allow", L"", SW_HIDE);
					//ShellExecute(NULL, L"open", L"netsh.exe", L"wlan start hostednetwork", L"", SW_HIDE);
					theApp.StartSoftAP(theApp.fileFlag, theApp.icsFlag, theApp.dsFlag);
					Sleep(1000);
				}
				*/
				AfxMessageBox(_T("Trans file error"));
				fclose(myMap[srcIP]->f);
				return;
			} 
			trace("open the file:%s",dstFilePath);
			myMap[srcIP]->filename = s;
			myMap[srcIP]->data[0] = ACK_TRANS_FILE_START;
			myMap[srcIP]->data[1] = '1';//accept
			SendTo(myMap[srcIP]->data, PKGCONTOFFSET, srcPort, srcIP);
			myMap[srcIP]->pkgCnt =0;
			
			msg.index = myMap[srcIP]->index;
			msg.percent = 0;
		    number = 6;
			mainDlg->SendMessage(WM_TEST, (WPARAM)&msg,(LPARAM) &number);

			break;
			/*
		case TRANS_FILE_CONT:
			if (NULL != myMap[srcIP]->f)
			{
				int rst = 0;
				int *pContLen = NULL;
			    unsigned int RecvPkgNo = 0;
				char md5_s[MD5_SIZE];
				char md5_dest[MD5_SIZE];
				char content[CONTENTSIZE];
				memset(md5_s,0,MD5_SIZE);
				memset(md5_dest,0,MD5_SIZE);
				memset(content,0,CONTENTSIZE);
				int  md5_is_true = 1;
				int len = 0;

				pContLen = (int *)&myMap[srcIP]->data[1];
				len = *pContLen;
				RecvPkgNo = *(int *)&myMap[srcIP]->data[10];	//the package No. which client is sending
				if(myMap[srcIP]->pkgCnt < RecvPkgNo) //we only ack the package we have received
					break;
				if(myMap[srcIP]->pkgCnt == RecvPkgNo)//this is the package I'm waiting for
				{
					memcpy_s(md5_s,MD5_SIZE,&myMap[srcIP]->data[PKGCONTOFFSET+(len)],MD5_SIZE);
			
				
					memcpy_s(content,len,&myMap[srcIP]->data[PKGCONTOFFSET],len);
			

					if(MD5Check(md5_s, content,(unsigned int) len )){
			

							rst = fwrite(&myMap[srcIP]->data[PKGCONTOFFSET], 1, len, myMap[srcIP]->f);
							myMap[srcIP]->mySize += rst; 
							mainDlg->m_progress.SetPos(myMap[srcIP]->data[5]);
							myMap[srcIP]->pkgCnt++;
							md5_is_true = 1;
					}
					else{
							md5_is_true = 0;
							
					}

				}
			
				
				myMap[srcIP]->data[0] = ACK_TRANS_FILE_CONT;
				memcpy_s(&myMap[srcIP]->data[1], sizeof(int), (char *)&md5_is_true, sizeof(int));
				memset(myMap[srcIP]->data+PKGCONTOFFSET, 0, sizeof(myMap[srcIP]->data)-PKGCONTOFFSET);
				SendTo(myMap[srcIP]->data, PKGCONTOFFSET, srcPort, srcIP);
			
			}
			break;
			*/
		case TRANS_FILE_END:
			trace("send file end msg");
			if (NULL != myMap[srcIP]->f)
			{
				fclose(myMap[srcIP]->f);
				myMap[srcIP]->f = NULL;
				myMap[srcIP]->data[0] = ACK_TRANS_FILE_END;
				memset(myMap[srcIP]->data+PKGCONTOFFSET, 0, sizeof(myMap[srcIP]->data)-PKGCONTOFFSET);
				SendTo(myMap[srcIP]->data, PKGCONTOFFSET, srcPort, srcIP);
			//	s.Format(_T("Received file successed %d bytes,"),myMap[srcIP]->mySize);
			//	AfxMessageBox(s);
			//	mainDlg->m_progress.SetPos(0);
				msg.index = myMap[srcIP]->index;
				msg.percent =0;
				number = 5;
				mainDlg->SendMessage(WM_TEST, (WPARAM)&msg,(LPARAM) &number);

				number = 11;
				ULONGLONG size = myMap[srcIP]->mySize;
			    mainDlg->SendMessage(WM_TEST, (WPARAM)&size,(LPARAM) &number);
  

			//	(CProgressCtrl * ) progress =(CProgressCtrl * )(mainDlg->GetDlgItem(IDC_PROGRESS1+myMap[srcIP]->index));
				myMap[srcIP]->pkgCnt = 0;
				/* disallow until disconnect from other ap
				if (srcIP == _T("192.168.137.1") || srcIP == _T("192.168.173.1"))
				{
					ShellExecute(NULL, L"open", L"netsh.exe", L"wlan set hostednetwork mode=allow", L"", SW_HIDE);
					//ShellExecute(NULL, L"open", L"netsh.exe", L"wlan start hostednetwork", L"", SW_HIDE);
					theApp.StartSoftAP(theApp.fileFlag, theApp.icsFlag, theApp.dsFlag);
					Sleep(1000);
				}
				*/
			}
			break;
		case CONNECT_TO_SERVER:
				trace("connect to server msg");
				rst = AfxMessageBox(_T("There's a connection request. Do you agree?"), MB_YESNO);
				if (IDYES  == rst)
				{
					myMap[srcIP]->data[1] = '1';
					rst = (int)myMap[srcIP]->data[1];
					for(int i=0; i<CLIENT_NUM; i++)
					{
						if (0 == clients[i].port)
						{						
							EnterCriticalSection(&m_clientChangeCond);
							clients[i].ip = srcIP;
							clients[i].port = srcPort;
							 MultiByteToWideChar(CP_ACP,
							0,
							(char *)(&myMap[srcIP]->data[PKGCONTOFFSET]),
							32,
							wbuf,
							64);
							clients[i].mac.Format(_T("%02x:%02x:%02x:%02x:%02x:%02x"),
								myMap[srcIP]->data[PKGCONTOFFSET+32], myMap[srcIP]->data[PKGCONTOFFSET+32+1],
								myMap[srcIP]->data[PKGCONTOFFSET+32+2], myMap[srcIP]->data[PKGCONTOFFSET+32+3],
								myMap[srcIP]->data[PKGCONTOFFSET+32+4], myMap[srcIP]->data[PKGCONTOFFSET+32+5]-1);
							clients[i].ssid.Format(_T("%s"), wbuf);
							s = clients[i].mac;
							LeaveCriticalSection(&m_clientChangeCond);
							EnterCriticalSection(&m_PeerChangeCond);
							rsnMac = s;
							wlanEvtType = WLAN_EVT_AP_PEER_ARRIVED;
							LeaveCriticalSection(&m_PeerChangeCond);
							SetEvent(hPeerChangeEvent);		
							break;
						}
					}
				}
				else
				{
					myMap[srcIP]->data[1] = '0';
				}
				myMap[srcIP]->data[0] = ACK_CONNECT_TO_SERVER;
				SendTo(myMap[srcIP]->data, 21, srcPort, srcIP);
			break;
		case TRANS_DISCONNECT:
				myMap[srcIP]->data[0] = ACK_TRANS_DISCONNECT;
				SendTo(myMap[srcIP]->data, PKGCONTOFFSET, srcPort, srcIP);
				number = 16;
				mainDlg->SendMessage(WM_TEST, (WPARAM)0,(LPARAM) &number);
				trace("disconnet the message");
				break;
	   case ACK_TRANS_DISCONNECT:
		    trace(" ack   disconnet the message ");
			SetEvent(hMutexDisconnect);
			break;


		case ACK_TRANS_FILE_START:
			trace("ack start send_start");
			theApp.TransFileAllowFlag = myMap[srcIP]->data[1];
        	SetEvent(myMap[srcIP]->hMutexFileStart);
			SetEvent(hMutexFileStart);
			//AfxMessageBox(_T("ack trans file start."));
			break;


			/*
		case ACK_TRANS_FILE_CONT:
			ackPkg = *(int *)&myMap[srcIP]->data[10];
			g_md5_is_true = *(int *)&myMap[srcIP]->data[1];
			SetEvent(hMutexFileCont);
			//AfxMessageBox(_T("ack trans file cont."));
			break;
			*/
		case ACK_TRANS_FILE_END:
			trace("ack send file ");
		    SetEvent(hMutexFileEnd);
			SetEvent(myMap[srcIP]->hMutexFileEnd);
			//AfxMessageBox(_T("ack trans file end."));
			break;
		case ACK_CONNECT_TO_SERVER:
			trace("ack connect server");
			theApp.connAllowFlag = myMap[srcIP]->data[1];
			SetEvent(hMutexConnectToServer);
		default:
			break;
		}
   }
   CAsyncSocket::OnReceive(nErrorCode);

}

BOOL CMyAsyncSocket::openFile(LPCTSTR filePath)
{
	CFileException ex;
	CString s;

	//if (!dstFile.Open(filePath,
    //  CFile::modeReadWrite | CFile::shareExclusive | CFile::typeBinary | CFile::modeCreate, &ex))
	{
		TCHAR szError[1024];

		ex.GetErrorMessage(szError, 1024);
		_tprintf_s(_T("Couldn't open source file: %1024s"), szError);
		s.Format(_T("open file %s failure"), filePath);
		AfxMessageBox(s);
		return false;
	}
	return true;

}

void CMyAsyncSocket::OnSend(int nErrorCode)
{
	 CAsyncSocket::OnSend(nErrorCode);
}

int CMyAsyncSocket::writeFile(CHAR *buf)
{
	CString s;
	s.Format(_T("write %s bytes"), buf);
	AfxMessageBox(s);
	//dstFile.Write(buf, sizeof(buf));
	//dstFile.Flush();
	
	return 0;
}

void CMyAsyncSocket::closeFile()
{
	//dstFile.Flush();
	//dstFile.Close();
}
