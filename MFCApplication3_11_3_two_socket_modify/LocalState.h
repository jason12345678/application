#pragma once


class CLocalState
{
public:
    CLocalState(void);
	CLocalState(LPVOID);
    ~CLocalState(void);
    static bool isRunning;
    enum STATE {DISCONNECT=0, CONNECTED,SERVICERUN, NORMAL, SENDFILEWAITACK, 
		SENDFILEACKCANCEL, SENDFILECANCEL, SENDFILEACKOK, SENDFILEACKEND, RECEIVINGFILE, RECEIVEFILEEND, RECEIVEFILECANCEL};
    void GoToConnected(CString SSID);
	void GotoNormal();
	void GotoDisconnect();
	void GotoServiceRun();
	void GotoSendFileWaitAck();
	void GotoSendFileAckCancel();
	void GotoSendFileCancel();
	void GotoSendFileAckOK();
	void GotoSendFileAckEnd();
	void GotoReceiveFileEnd();
	void GotoReceiveFileCancel();
	void GotoReceivingFile();
    STATE GetCurrentState();
    CString GetConnectedSSID();
	void SetParent(LPVOID);
private:
    STATE LocalState;
    CString ConnectedPeerSSID;
	void * m_pParentDialog;
};

