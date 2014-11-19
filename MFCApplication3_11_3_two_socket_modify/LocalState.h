#pragma once


class CLocalState
{
public:
    CLocalState(void);
	CLocalState(LPVOID);
    ~CLocalState(void);
    static bool isRunning;
    enum STATE {DISCONNECT=0, CONNECTED,SERVICERUN};
    void GoToConnected(CString SSID);
	void GotoDisconnect();
	void GotoServiceRun();
	void GotoSendFileWaitAck();
	void GotoSendFileAckCancel();
	void GotoSendFileAckOK();
	void GotoSendFileAckEnd();
    STATE GetCurrentState();
    CString GetConnectedSSID();
	void SetParent(LPVOID);
private:
    STATE LocalState;
    CString ConnectedPeerSSID;
	void * m_pParentDialog;
};

