
// MFCApplication3.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "afxwin.h"
#include "MyAsyncSocket.h"
#include "afxcmn.h"
#include <windows.h>
#include <objbase.h>
#include <netcon.h>
#include <stdio.h>
#include <NetCon.h>
#include <locale.h>
#include <wlanapi.h>
#include <WinNetWk.h>
#include <netlistmgr.h>
#include <ShObjIdl.h>
#pragma comment (lib, "wlanapi.lib")
#pragma comment (lib,"Mpr.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

#include "resource.h"		// 主符号


#define CLIENT_NUM 10

// SmartSharingApp:
// 有关此类的实现，请参阅 MFCApplication3.cpp
//

class SmartSharingApp : public CWinApp
{
public:
	SmartSharingApp();
	~SmartSharingApp();
	static DWORD WINAPI StartSoftAP(BOOL fileTransFlag=False, BOOL icsFlag=False, BOOL sharDskFlag=False);
    static DWORD WINAPI StopSoftAP();
    static DWORD WINAPI ScanAllBssInfo(_In_  LPVOID lpParameter);
    static DWORD WINAPI ConnectToAP(_In_  LPVOID lpParameter);
	void startICS();
	void stopICS();
	bool setSoftApAddr(CString wlanIP=NULL, CString softApIP=NULL);

// 重写
public:
	char connAllowFlag;
	char TransFileAllowFlag;
	virtual BOOL InitInstance();
public:
	int fileFlag;
	int icsFlag;
	int dsFlag;
	CString wlanName;
	CString virtualName;

// 实现

	DECLARE_MESSAGE_MAP()
};

typedef struct __ClientInfo
{
	CString ssid;
	CString ip;
	CString mac;
	UINT port;

} ClientInfo;

extern CRITICAL_SECTION m_clientChangeCond;
extern ClientInfo clients[CLIENT_NUM];
extern SmartSharingApp theApp;
extern	CString strSSID;
extern BYTE macAddr[6];