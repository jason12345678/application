#pragma once

// CMyAsyncSocket 命令目标

#include "common.h"

class CMyAsyncSocket : public CAsyncSocket 
{
public:
	CMyAsyncSocket();
	virtual ~CMyAsyncSocket();
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	BOOL openFile(LPCTSTR filePath);
	int writeFile(CHAR *buf);
	void closeFile();

	

private:
	CFile dstFile;
	
};


