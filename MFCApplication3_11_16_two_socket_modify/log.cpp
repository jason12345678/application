#include "stdafx.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <stdarg.h>
#include <time.h>


extern int g_bDebug;

#pragma warning(disable:4996)

void trace( const char *fmt,...)
{
	FILE *log_fp;


	if(!g_bDebug)
		return;

	if( (log_fp = fopen("c:\\application.log","a+")) == NULL  ){
		perror("open c:\\application.log err");
		return ;
	}
	va_list args;
	va_start(args, fmt);
	time_t now;
	time(&now);
	char ch_time[30];
	struct tm* time;
	time = localtime(&now);
	sprintf(ch_time,"%04d-%02d-%02d %02d:%02d:%02d",time->tm_year+1900,time->tm_mon+1,time->tm_mday, time->tm_hour,time->tm_min,time->tm_sec);
	fprintf(log_fp,"%s,",ch_time);		
	vfprintf(log_fp,fmt,args);
	fprintf(log_fp,"\n");
	va_end(args);
	fclose(log_fp);
}



int get_key()
{
	// TODO: Add your control notification handler code here
		CString value = CString("SOFTWARE\\application");		
		char Buffer[256];
		DWORD dwType = REG_SZ;	
		DWORD dwSize = sizeof(Buffer);
		HKEY hKey;	RegOpenKey(HKEY_LOCAL_MACHINE, value, &hKey);	
		RegQueryValueEx(hKey, _T("debug"), 0, &dwType ,(LPBYTE)Buffer, &dwSize);
		CString str;	
		str.Format(_T("%s"),Buffer);       
		if(str==CString(_T("1")))
			return 1;
		else
			return 0;
		
}

int set_key()
{

	CString value =CString( "SOFTWARE\\application");	
	CString name = CString("debug");   
	HKEY hKey;   
	RegCreateKey(HKEY_LOCAL_MACHINE,value,&hKey);
	TCHAR * szValue = _T("1"); 
	RegSetValueEx(hKey, name, 0, REG_SZ, (BYTE*)szValue, (_tcslen(szValue)+1)*sizeof(TCHAR));
	RegCloseKey(hKey);
	return 0;
}