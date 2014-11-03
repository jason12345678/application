#pragma once

#include <Windows.h>

#include <vector>
#include <atlbase.h>
extern CComModule _Module;      // for receiving events from COM server
#include <atlcom.h>
#include <atlctl.h>
#include <atlstr.h>

#include <wlanapi.h>
#include <iphlpapi.h>
//#include <GuidDef.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wlanapi.lib")
#pragma comment(lib, "IPHLPAPI.lib")


// Import Direct Display COM server's type library 
#import "DirectDisplayServer.tlb" no_namespace

#define IDC_SRCOBJ    1
#define MAX_DISPLAY 16      

#define QCA_MAX_IOCTL_BUFFER		65536
#define WIN_DEV_INST_PREFIX _T("\\\\?\\")

#define ATH_JANUS_ID        _T("VEN_168C")
#define ATH_JANUS_USB_ID    _T("VID_0CF3&PID_9271")
#define ATH_SDIO_ID         _T("VID_0271&PID_0301")

#define OID_ATH_SMARTANT_INVITE                 0xFFA30013
/*
#define IOCTL_NDIS_QUERY_GLOBAL_STATS	\
CTL_CODE(FILE_DEVICE_PHYSICAL_NETCARD, 0, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)*/

#define INFORMATION_ELEMENT_SIZE 128 // qca nic used buffer size = 128 bytes
#define QUALCOMM_WIFI_DISPLAY_ANSI_IDENTIFICATION  "Qualcomm_Atheros_Miracast_V001"  // must be ascii char
#define SMARTSHARING_SSID_PREFIX "SmartSharing"

//extern HWND g_hWnd = NULL;
void SetHWND(HWND hWnd);

#define WM_WFD_CALLBACK_MSG (WM_USER + 500)

#define GUID_STR_LEN 39

typedef enum _WFD_ERROR_CODE
{
    WFD_ERROR_SUCCESS,
    WFD_ERROR_INVALID_PARAMS, 
    WFD_ERROR_COM_UNINITIALIZED,
    WFD_ERROR_INIT_COM_FAILED,
    WFD_ERROR_START_SESSION_FAILED,
    WFD_ERROR_STOP_SESSION_FAILED,
    WFD_ERROR_PLAY_FAILED,
    WFD_ERROR_PAUSE_FAILED,

    WFD_ERROR_HOSTEDNETWORK_QUERY_STATE_FAILED,
    WFD_ERROR_HOSTEDNETWORK_SET_PROPERTY_FAILED,
    WFD_ERROR_HOSTEDNETWORK_NIC_NOT_ENABLE,
    WFD_ERROR_HOSTEDNETWORK_INVALID_STATUS,
    WFD_ERROR_HOSTEDNETWORK_ALLOW_FAILED,        
    WFD_ERROR_HOSTEDNETWORK_START_FAILED,
    WFD_ERROR_HOSTEDNETWORK_GET_IP_FAILED,
    WFD_ERROR_HOSTEDNETWORK_STOP_FAILED,

    WFD_ERROR_WLAN_ACQUIRE_HANDLE_FAILED,
    WFD_ERROR_WLAN_NO_VALID_INTERFACE,
    WFD_ERROR_WLAN_REGISTER_MESSAGE_HANDLER_FAILED,

    WFD_ERROR_WLAN_SCAN_FAILED,
    WFD_ERROR_WLAN_SCAN_TIMEOUT,
    WFD_ERROR_WLAN_GET_BSS_LIST_FAILED,

    WFD_ERROR_WLAN_CONNECT_FAILED,
    WFD_ERROR_WLAN_CONNECT_TIMEOUT,
    WFD_ERROR_WLAN_GET_IP_FAILED,

    WFD_ERROR_WLAN_SET_ATHEROS_NIC_IE_FAILED,

    WFD_ERROR_OTHER,    

	WFD_ERROR_INVALID_HANDLE=200,
	WFD_ERROR_INVALID_PARAMETER,
	WFD_ERROR_NDIS_DOT11_POWER_STATE_INVALID,
	WFD_ERROR_NOT_ENOUGH_MEMORY,
	WFD_ERROR_NOT_FOUND,
	WFD_ERROR_NOT_SUPPORTED,
	WFD_ERROR_SERVICE_NOT_ACTIVE,
	WFD_RPC_STATUS
}WFD_ERROR_CODE;


typedef HRESULT (__stdcall * EventHandlerProc)(CString strSessionId, LONG eventType, ULONG hresult);



class CEventHandler : public IDispEventImpl<IDC_SRCOBJ, CEventHandler,
    &__uuidof(_IDirectDisplaySourceEvents), &__uuidof(__DirectDisplayServerLib), 1, 0>
{
public:
    BEGIN_SINK_MAP(CEventHandler)
        SINK_ENTRY_EX(IDC_SRCOBJ, __uuidof(_IDirectDisplaySourceEvents), 1, OnDirectDisplayEvent)
    END_SINK_MAP()

    HRESULT __stdcall OnDirectDisplayEvent(BSTR bstrSessionId, enum tag_dds_event eventType, ULONG hresult);
};



class CSinkEventHandler : public IDispEventImpl<IDC_SRCOBJ, CSinkEventHandler,
    &__uuidof(_IDirectDisplaySinkEvents), &__uuidof(__DirectDisplayServerLib), 1, 0>
{
public:
    BEGIN_SINK_MAP(CSinkEventHandler)
        SINK_ENTRY_EX(IDC_SRCOBJ, __uuidof(_IDirectDisplaySinkEvents), 1, OnDirectDisplaySinkEvent)
    END_SINK_MAP()

    HRESULT __stdcall OnDirectDisplaySinkEvent(BSTR bstrSessionId, enum tag_dds_event eventType, ULONG hresult);
};




class CWfdInterfaceSolution
{    
public:
    CWfdInterfaceSolution();
    ~CWfdInterfaceSolution();

    WFD_ERROR_CODE CreateDirectDisplaySourceObject();
    WFD_ERROR_CODE ReleaseDirectDisplaySourceObject();

    WFD_ERROR_CODE SetVideoConfig(
        long lVideoEncoderType,                                                      
        long lVideoEncoderHwAccelEnable, 
        long lVideoModeAutoSelect, 
        long lVideoModeType, 
        unsigned long ulVideoModeIndex, 
        RECT rcCaptureRect
        );

    WFD_ERROR_CODE SetAudioConfig(long lAudioEncoderType);
    WFD_ERROR_CODE SetMiscellaneousConfig(CString & strNetworkInterface);    

    WFD_ERROR_CODE StartSession();
    WFD_ERROR_CODE StopSession();
    WFD_ERROR_CODE Play();
    WFD_ERROR_CODE Stop();

    WFD_ERROR_CODE GetSinkList(CString & strSinkList);
    WFD_ERROR_CODE TearDownSink(CString & strSinkIpAddress);

    WFD_ERROR_CODE RegisterCallbacks(EventHandlerProc pEventHandlerProc);

    WFD_ERROR_CODE TestFunction();


    
    // display number tip function    
    WFD_ERROR_CODE RefreshDisplayNum(int & nDisplayCount);
    static int m_displayCount;
    static HMONITOR m_displayHmonitor[MAX_DISPLAY];
    static RECT m_displayRect[MAX_DISPLAY];
    static BOOL CALLBACK MonitorEnumProc(
        __in  HMONITOR hMonitor, 
        __in  HDC hdcMonitor, 
        __in  LPRECT lprcMonitor, 
        __in  LPARAM dwData
        );
    
private:
    CComPtr<IDirectDisplaySource> m_pDD; // Direct Display source interface pointer
    CEventHandler m_eventHandler;       // Event handler for COM server events
};




class CWfdInterfaceSolutionSink
{
public:
    CWfdInterfaceSolutionSink();
    ~CWfdInterfaceSolutionSink();

    WFD_ERROR_CODE CreateDirectDisplaySourceObject();
    WFD_ERROR_CODE ReleaseDirectDisplaySourceObject();

    WFD_ERROR_CODE SetConfig(CString & strSourceIpAddress, CString & strUserSpecificName);

    WFD_ERROR_CODE StartSession();
    WFD_ERROR_CODE StopSession();
    WFD_ERROR_CODE Play();
    WFD_ERROR_CODE Stop();

    WFD_ERROR_CODE RegisterCallbacks(EventHandlerProc pEventHandlerProc);
private:
    // sink (public)
    CComPtr<IDirectDisplaySink> m_pDDSink;  // Direct Display Sink interface pointer
    // Sink
    CSinkEventHandler m_SinkEventHandler;   // Event handler for COM server events   
};

typedef enum _wlanEvtType
{
	WLAN_EVT_INVAILD,
	WLAN_EVT_AP_PEER_ARRIVED,
	WLAN_EVT_AP_PEER_DEPARTED,
	WLAN_EVT_STA_DISCONNECT
} WLANEVTTYPE;

class CWfdWlanSolution
{
public:
    CWfdWlanSolution(void);
    ~CWfdWlanSolution(void);  

    static CWfdWlanSolution * Instance()
    {
        static CWfdWlanSolution inst;
        return &inst;
    }

	BYTE* getWifiCardMacAddr(BYTE * VirtualAdapterAddress);
	bool getWifiNames(char* pStrWlanAdatperName, char* pStrHostNetworkAdapterName);
	

    WFD_ERROR_CODE HostedNetworkStartSoftAp(IN CString strSSID, IN CString strSecondaryKey, OUT in_addr& ip_local_address);    
    WFD_ERROR_CODE HostedNetworkStopSoftAp();
    WFD_ERROR_CODE ScanBssList(std::vector<WLAN_BSS_ENTRY> & vecSelectedBSSEntry, bool bZero = true,int matchType=1);//matchType=0 match vendoerIE, 1 match SSID
    WFD_ERROR_CODE ConnectToAP(IN unsigned int nBssEntryIndex, IN CString strSecondaryKey, OUT in_addr & ip_local_address, OUT in_addr & ip_gateway_address);
    static WFD_ERROR_CODE SetAtherosNICInfomationElement(bool bIsSetSepcificInfo);  
    bool isPasswordNeeded(int nIndex);
	WFD_ERROR_CODE DisConnectToAP();
	bool IsSameGUID(char* a, GUID* b);
	void GuidToString(GUID* Guid, char* GUIDString);
	WFD_ERROR_CODE EnumWlanAdapterInterfaceGUID();
	WFD_ERROR_CODE EnumVirtualAdapterInterfaceGUID();
	WFD_ERROR_CODE getWlanAdapterGUID(char*);
	WFD_ERROR_CODE getVirtualAdapterGUID(char*);

private:
    // - tip functions - 
    WFD_ERROR_CODE CheckWlanHandle();   
    WFD_ERROR_CODE RegisterNotificationFunc();
    static VOID NotificationCallback(
		__in PWLAN_NOTIFICATION_DATA pNotifData, 
		__in_opt PVOID pContext
		);

    
    // - hosted network -
    WFD_ERROR_CODE HostedNetworkQueryState(int & state);
    WFD_ERROR_CODE HostedNetworkSetMode(BOOL bIsAllow);
    WFD_ERROR_CODE HostedNetworkUpdateIp(in_addr & hostednetworkip);
    

    // - scan -
    int SetScanConditionCriticalSectionValue(unsigned int nWlanScanCondition);
    unsigned int GetScanConditionCriticalSectionValue();
    int SetScanResultCriticalSectionValue(unsigned int nWlanScanResult);
    unsigned int GetScanResultCriticalSectionValue();
    WFD_ERROR_CODE ChooseUsedInterface();

    

    // - connect - 
    int SetConnectConditionCriticalSectionValue(unsigned int nWlanScanCondition);
    unsigned int GetConnectConditionCriticalSectionValue();
    WFD_ERROR_CODE GetSoftAPTemporaryProfile(CString strSSID ,CString strSecondaryKey, CString & strPrifile);    
    WFD_ERROR_CODE UpdateIPAddress();

    

    // - NIC issue -
    static DWORD 
	AthEnumQcaWiFiDevices( 
		__out		LPTSTR      DevPaths,
		__in __out	PULONG		pPathLength,
		__out		PULONG      pNumDevFound
		);

	static DWORD 
	AthOpenWiFiDevice(
		__in		LPCTSTR     DevPath,
		__in __out  PHANDLE     pDeviceHandle
		);

	static DWORD
	AthCloseWiFiDevice(
		__in    HANDLE      hWiFiDevice
		);

	static DWORD
	AthWiFiDeviceIoControl(
		__in    HANDLE      hWiFiDevice,
		__in    DWORD       dwOidIoControlCode,
		__in    LPVOID      lpInBuffer,
		__in    ULONG       lInBufferSize,
		__out   LPVOID      lpOutBuffer,
		__out   ULONG       lOutBufferSize,
		__in    ULONG       lFlags,
		__out   PULONG      pBytesReturned
		);

	static BOOL
	AthIsQcaWiFiDevice(
		__in    LPTSTR		DevPath
		);

	static DWORD
	SplitMultiString(
		__in    LPCTSTR     MultiString,        
		__out   LPTSTR		**StringArray,
		__out   PULONG      NumOfStringReturned
		);

	static VOID
	MakeStrUpperCase( 
		__in __out  LPTSTR  DevPath 
		);	
        

private:        
    HANDLE m_hWlanClient;
    // scan
    unsigned int m_nWlanScanCondition;  // 0- scanning; 1 -scan finished; 
    CRITICAL_SECTION m_csWlanScanCondition;
    unsigned int m_nWlanScanResult;     // 0 - scan success; 1 - scan failed.    
    CRITICAL_SECTION m_csWlanScanResult;
    WLAN_INTERFACE_INFO m_usedWlanInterface;
	char m_WlanInterfaceGUID[GUID_STR_LEN];
	char m_VirtualAdapterGUID[GUID_STR_LEN];

    std::vector<WLAN_BSS_ENTRY> m_vectorSelectedBSSEntry;
    
    // connect
    unsigned int m_nWlanConnectCondition; // 0 - unconnect, 1 - connected
    CRITICAL_SECTION m_csWlanConnectCondition;
    in_addr m_WifiConnectedGatewayIP;
    in_addr m_WifiConnectedLocalIP;
};

