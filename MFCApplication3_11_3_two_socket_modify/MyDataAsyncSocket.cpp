// MyAsyncSocket.cpp : 实现文件
//

#include "stdafx.h"
#include "LocalState.h"
#include "SmartSharingApp.h"
#include "MyDataAsyncSocket.h"
#include "SmartSharingDlg.h"
#include "SendFileTipDlg.h"
#include "md5_check.h"
#include "log.h"
#include <map>
using namespace std;


extern unsigned int ackPkg = 0;
extern int g_md5_is_true = 0;

extern map<CString ,client_data *  >myMap;

CMyDataAsyncSocket::CMyDataAsyncSocket()
{

}

CMyDataAsyncSocket::~CMyDataAsyncSocket()
{
}

// CMyAsyncSocket 成员函数
void CMyDataAsyncSocket::OnReceive(int nErrorCode)
{
	BYTE revPkg[PKGSIZE] = {0};
	
	int nRead;
	CString fileName;
	int rst;
	CString pszFileName;
	char tmp[FILE_NAME_LENGTH] = {0};
	char dstFilePath[FILE_NAME_LENGTH] = {0};
	CString srcIP;
	UINT srcPort = 500;
	CString s;
	CFileDialog *dlgFile = NULL;
	char *pPos = NULL;
	CString suffix;
	CString defName;
	CSendFileTipDlg dlg;
	char tmpSSID[32] = {0};
	wchar_t wbuf[100] = {0};

	memset(revPkg, 0, sizeof(revPkg));
	nRead = ReceiveFrom(revPkg, sizeof(revPkg), srcIP, srcPort);

	if(myMap[srcIP]==NULL){
		/*
		client_data *pData = new client_data();
		memcpy_s(pData->data, sizeof(revPkg),revPkg,sizeof(revPkg));
		pData->mySize = 0;
		myMap[srcIP] =pData;
		*/
		s.Format(_T("not exited the IP address"));
		AfxMessageBox (s);
		return ;

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
		
		switch (revPkg[0])
		{
	
		case TRANS_FILE_CONT:
			if (NULL != myMap[srcIP]->f)
			{
				trace("begin to receive the data");
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
			  
				int number;
				number = 15;
			    mainDlg->SendMessage(WM_TEST, (WPARAM)0,(LPARAM) &number);

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
							//mainDlg->m_progress.SetPos(myMap[srcIP]->data[5]);
							message msg;
							msg.index = myMap[srcIP]->index;
							msg.percent = myMap[srcIP]->data[5];
							int number = 5;
							mainDlg->SendMessage(WM_TEST, (WPARAM)&msg,(LPARAM) &number);
							myMap[srcIP]->pkgCnt++;
							md5_is_true = 1;
					}
					else{
							md5_is_true = 0;
							
					}

				}
				//if the pkgCnt > RecvPkgNo, it means we have received this package, ack it but do not change current state
				
				myMap[srcIP]->data[0] = ACK_TRANS_FILE_CONT;
				memcpy_s(&myMap[srcIP]->data[1], sizeof(int), (char *)&md5_is_true, sizeof(int));
				memset(myMap[srcIP]->data+PKGCONTOFFSET, 0, sizeof(myMap[srcIP]->data)-PKGCONTOFFSET);
				SendTo(myMap[srcIP]->data, PKGCONTOFFSET, srcPort, srcIP);
			
			}
			break;
	
		case ACK_TRANS_FILE_CONT:
			trace("ack send file cont");
			ackPkg = *(int *)&myMap[srcIP]->data[10];
			g_md5_is_true = *(int *)&myMap[srcIP]->data[1];
		//	myMap[srcIP]->md5_is_true = *(int *)&myMap[srcIP]->data[1];
		//	myMap[srcIP]->ackPkg = *(int *)&myMap[srcIP]->data[10];
			SetEvent(hMutexFileCont);
		//	SetEvent(myMap[srcIP]->hMutexFileCont);
			//AfxMessageBox(_T("ack trans file cont."));
			break;
		default:
			break;
		}
   }
   CAsyncSocket::OnReceive(nErrorCode);

}

BOOL CMyDataAsyncSocket::openFile(LPCTSTR filePath)
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

void CMyDataAsyncSocket::OnSend(int nErrorCode)
{
	 CAsyncSocket::OnSend(nErrorCode);
}

int CMyDataAsyncSocket::writeFile(CHAR *buf)
{
	CString s;
	s.Format(_T("write %s bytes"), buf);
	AfxMessageBox(s);
	//dstFile.Write(buf, sizeof(buf));
	//dstFile.Flush();
	
	return 0;
}

void CMyDataAsyncSocket::closeFile()
{
	//dstFile.Flush();
	//dstFile.Close();
}

