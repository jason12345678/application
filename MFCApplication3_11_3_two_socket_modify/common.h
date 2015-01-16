#pragma once

#define PKGSIZE 1024 * 63 + 16+33
#define PKGCONTOFFSET 16
#define CONTENTSIZE  1024 *63
#define MD5_SIZE 33
#define SOCK_PORT_RECV 10000
#define SOCK_PORT_SEND 10001


#define SOCK_PORT_RECV_DATA 10005


#define SOCK_PORT_RECV_SERVER   9122
#define SOCK_PORT_SEND1			10008


#define SOCK_PORT_RECV_SERVER_DATA 10010

#define WAIT_ACK_TIME_OUT 1000
#define FILE_NAME_LENGTH 500

#define TRANS_FILE_START 0x1
#define TRANS_FILE_CONT 0x2
#define TRANS_FILE_END 0x3
#define TRANS_FILE_CANCEL 0x11

#define ACK_TRANS_FILE_START 0x4
#define ACK_TRANS_FILE_CONT 0x5
#define ACK_TRANS_FILE_END 0x6
#define CONNECT_TO_SERVER 0x7
#define ACK_CONNECT_TO_SERVER 0x8

#define TRANS_DISCONNECT 0x09
#define ACK_TRANS_DISCONNECT 0x10



#define WINCAPSERVER        L"\\app_refs_ut_capserver.exe"
#define WINPLAYER           L"\\app_refs_ut_simpleplayer.exe"
#define WINCAPSERVERNAME    L"app_refs_ut_capserver.exe"
#define WINPLAYERNAME       L"app_refs_ut_simpleplayer.exe"


typedef struct _pkg
{
	BYTE cmd;
	CHAR content[PKGSIZE];
} SockPkg;

typedef struct _client_data {
	FILE *f;
	BYTE data[PKGSIZE];
	CString filename;
	int index;
	ULONGLONG mySize;
	unsigned int pkgCnt;
	int  md5_is_true;
	unsigned int ackPkg;
	int mySendSize;
	bool send_flag;
	HANDLE hMutexFileStart;
	HANDLE hMutexFileEnd;
	HANDLE hMutexFileCont;
}client_data;

typedef struct _message {

	int index;
	int percent;

}message;

typedef struct _dragmsg{

	int index;
	CString filepath;

}dragmsg;
