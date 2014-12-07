
#include "stdafx.h"

#include <winioctl.h>
#include <ntddndis.h>

CComModule _Module;
extern __declspec(selectany) CAtlModule* _pAtlModule=&_Module;

HWND g_hWnd = NULL;
void SetHWND(HWND hWnd)
{
    g_hWnd = hWnd;
}
// - Event Handler-

// Event handler (source)
HRESULT __stdcall CEventHandler::OnDirectDisplayEvent(BSTR bstrSessionId, enum tag_dds_event eventType, ULONG hresult)
{ 
    if (NULL == g_hWnd)
    {
        return S_OK;
    }
    CString strSessionId;
    strSessionId = bstrSessionId;
    SysFreeString(bstrSessionId);
    PostMessage(g_hWnd, WM_WFD_CALLBACK_MSG, eventType, hresult);
    return S_OK;
}

// Event handler (sink)
HRESULT __stdcall CSinkEventHandler::OnDirectDisplaySinkEvent(BSTR bstrSessionId, enum tag_dds_event eventType, ULONG hresult)
{
    if (NULL == g_hWnd)
    {
        return S_OK;
    }
    CString strSessionId;
    strSessionId = bstrSessionId;
    SysFreeString(bstrSessionId);
    PostMessage(g_hWnd, WM_WFD_CALLBACK_MSG, eventType, hresult);
    return S_OK;
}

// - CWfdInterfaceSolution -

int CWfdInterfaceSolution::m_displayCount = 0;
HMONITOR CWfdInterfaceSolution::m_displayHmonitor[MAX_DISPLAY] = {0};
RECT CWfdInterfaceSolution::m_displayRect[MAX_DISPLAY];

CWfdInterfaceSolution::CWfdInterfaceSolution()
{
    m_pDD = NULL;
}


CWfdInterfaceSolution::~CWfdInterfaceSolution()
{
    ReleaseDirectDisplaySourceObject();
}


WFD_ERROR_CODE CWfdInterfaceSolution::CreateDirectDisplaySourceObject()
{
    // Create Direct Display Source object
    CComPtr<IUnknown> spUnknown;
    spUnknown.CoCreateInstance(__uuidof(DirectDisplaySource));

    if (!spUnknown) {
        return WFD_ERROR_INIT_COM_FAILED;
    } 

    // Get Direct Display Source interface
    spUnknown.QueryInterface(&m_pDD);

    if (!m_pDD) {
        return WFD_ERROR_INIT_COM_FAILED;
    } 

    //m_eventHandler.SetEventHandlerFunction(NULL);

    // Start listen to events from Direct Display Source
    m_eventHandler.DispEventAdvise(m_pDD);
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdInterfaceSolution::ReleaseDirectDisplaySourceObject()
{
    // Stop listening to events from Direct Display COM server
    m_eventHandler.DispEventUnadvise(m_pDD);
    //m_eventHandler.SetEventHandlerFunction(NULL);

    if (m_pDD) {
        // Must release the interface before uninitilizing COM.
        // We cannot wait for the member destruction to release the interface.
        m_pDD.Release();
        m_pDD = NULL;
    }
    return WFD_ERROR_SUCCESS;
}



WFD_ERROR_CODE CWfdInterfaceSolution::SetVideoConfig(
    long lVideoEncoderType,                                                      
    long lVideoEncoderHwAccelEnable, 
    long lVideoModeAutoSelect, 
    long lVideoModeType, 
    unsigned long ulVideoModeIndex, 
    RECT rcCaptureRect)
{
    if (NULL == m_pDD)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }
    // Source video encoder type
    if (lVideoEncoderType < 0 || lVideoEncoderType >= tag_dds_video_encoder_type::DDS_VIDEO_ENCODER_MAX)
    {
        lVideoEncoderType = tag_dds_video_encoder_type::DDS_VIDEO_ENCODER_AVC;
    }
    m_pDD->PutVideoEncoderType(static_cast<enum tag_dds_video_encoder_type>(lVideoEncoderType));

    // Source video encoder hw accel enable
    if (lVideoEncoderHwAccelEnable < 0 || lVideoEncoderHwAccelEnable > 1)
    {
        lVideoEncoderHwAccelEnable = 1;
    }
	m_pDD->PutVideoEncoderHwAccelEnable(lVideoEncoderHwAccelEnable);

    // Initial Video Mode Selection (if not set to auto - default setting)
    if(lVideoModeAutoSelect < 0 || lVideoModeAutoSelect > 1)
    {
        lVideoModeAutoSelect = 0;
    }
    m_pDD->PutVideoModeAutoSelect(lVideoModeAutoSelect);

    if(lVideoModeType < 0 || lVideoModeType > tag_dds_videoDisplay::DDS_hh)
    {
        lVideoModeType = 0;
    }
    m_pDD->PutVideoModeType(static_cast<enum tag_dds_videoDisplay>(lVideoModeType));

    m_pDD->PutVideoModeIndex(ulVideoModeIndex);

    struct tag_dds_rect rect;
    rect.bottom = rcCaptureRect.bottom;
    rect.left = rcCaptureRect.left;
    rect.right = rcCaptureRect.right;
    rect.top = rcCaptureRect.top;
    if(0 == rect.bottom 
        || 0 == rect.right
        || rect.bottom <= rect.top
        || rect.right <= rect.left
        || rect.right > GetSystemMetrics(SM_CXSCREEN)
        || rect.bottom > GetSystemMetrics(SM_CYSCREEN))
    {
        rect.left = rect.top = 0;
        rect.right = GetSystemMetrics(SM_CXSCREEN);
        rect.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    m_pDD->PutCaptureRect(rect);

    return WFD_ERROR_SUCCESS;
}



WFD_ERROR_CODE CWfdInterfaceSolution::SetAudioConfig(long lAudioEncoderType)
{
    if (NULL == m_pDD)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }

    // Audio encoder type
    if(lAudioEncoderType < 0 || lAudioEncoderType >= tag_dds_audio_encoder_type::DDS_AUDIO_ENCODER_MAX)
    {
        lAudioEncoderType = tag_dds_audio_encoder_type::DDS_AUDIO_ENCODER_AAC;
    }
    m_pDD->PutAudioEncoderType(static_cast<enum tag_dds_audio_encoder_type>(lAudioEncoderType));
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdInterfaceSolution::SetMiscellaneousConfig(CString & strNetworkInterface)
{
    if (NULL == m_pDD)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }

    // Set Source IP address (Network interface)
    if (strNetworkInterface.GetLength() <= 0 )
    {
        strNetworkInterface = _T("127.0.0.1");
    }
    _bstr_t bstrNetworkInterface = strNetworkInterface;
    m_pDD->PutSourceIpAddress(bstrNetworkInterface);

    _bstr_t bstrIpAddress(L""); // if you wanna try a rtp stream without rtsp ,just replace PutDestIpAddress with the target
    m_pDD->PutCoupledSinkEnable(0);
    m_pDD->PutDestIpAddress(bstrIpAddress);

    // Source session management type (SDP/RTSP) - RTSP
    m_pDD->PutSessionManagementType(static_cast<enum tag_dds_session_mgmt_type>(DDS_SESSION_MGMT_RTSP));

    // Source RTSP port (only used if session type is RTSP)
    m_pDD->PutRtspPort(7236);

    // Source transport type (UDP/RTP)
    m_pDD->PutTransportType(static_cast<enum tag_dds_transport_type>(DDS_TRANSPORT_RTP));

    // Set Config file path
    CString strConfigFilePath;
    // Get application path
    GetModuleFileName(NULL, strConfigFilePath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
    int i = strConfigFilePath.ReverseFind('\\') ; // find first backslash from end
    strConfigFilePath = strConfigFilePath.Left(i + 1) ;
    // Test if "application path\DDServerFiles" exists.
    // If so, use this, otherwise just use the application path
    struct _stat buf;
    if (_tstat(strConfigFilePath + _T("DDServerFiles"), &buf) == 0)
    {
        strConfigFilePath += _T("DDServerFiles");
        strConfigFilePath += "\\";
    }
    _bstr_t bstrConfigFilePath(strConfigFilePath);
    m_pDD->PutConfigFilePath(bstrConfigFilePath);

    // UIBC feature enable
    m_pDD->PutUIBCFeatureEnable(0);

    return WFD_ERROR_SUCCESS;
}



WFD_ERROR_CODE CWfdInterfaceSolution::StartSession()
{
    if (NULL == m_pDD)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }

    try
    {
        // Start session
        m_pDD->StartSession();
    }
    catch (_com_error& e) {
        return WFD_ERROR_START_SESSION_FAILED;
    }
    return WFD_ERROR_SUCCESS;
}



WFD_ERROR_CODE CWfdInterfaceSolution::StopSession()
{
    if (NULL == m_pDD)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }

    try 
    {
        // Stop session
        m_pDD->StopSession();
    }
    catch (_com_error& e) {
        return WFD_ERROR_STOP_SESSION_FAILED;
    }
    return WFD_ERROR_SUCCESS;
}


WFD_ERROR_CODE CWfdInterfaceSolution::Play()
{
    if (NULL == m_pDD)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }
    try 
    {
        // Play
        m_pDD->Play();
    }
    catch (_com_error& e) {
        return WFD_ERROR_PLAY_FAILED;
    }
    return WFD_ERROR_SUCCESS;
}


WFD_ERROR_CODE CWfdInterfaceSolution::Stop()
{
    if (NULL == m_pDD)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }

    try 
    {
        // Stop
        m_pDD->Stop();
    }
    catch (_com_error& e) {
        return WFD_ERROR_PAUSE_FAILED;
    }
    return WFD_ERROR_SUCCESS;
}


WFD_ERROR_CODE CWfdInterfaceSolution::GetSinkList(CString & strSinkList)
{
    if (NULL == m_pDD)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }    
    _bstr_t sink_list;
    sink_list = m_pDD->GetSinkList();
    strSinkList = (TCHAR*)sink_list;
    SysFreeString(sink_list.GetBSTR());    
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdInterfaceSolution::TearDownSink(CString & strSinkIpAddress)
{
    if (NULL == m_pDD)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }

    if (strSinkIpAddress.GetLength() <= 0)
    {
        strSinkIpAddress = _T("127.0.0.1");
    }

    _bstr_t ip_address(strSinkIpAddress);
    m_pDD->TeardownSink(ip_address);
    return WFD_ERROR_SUCCESS;
}


WFD_ERROR_CODE CWfdInterfaceSolution::RegisterCallbacks(EventHandlerProc pEventHandlerProc)
{
    
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdInterfaceSolution::TestFunction()
{
    MessageBox(NULL,_T("Test"),_T("Test"),MB_OK);
    return WFD_ERROR_SUCCESS;
}


BOOL CWfdInterfaceSolution::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    if (m_displayCount < MAX_DISPLAY)
    {
        m_displayHmonitor[m_displayCount] = hMonitor;
        m_displayRect[m_displayCount] = *lprcMonitor;
        m_displayCount++;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

WFD_ERROR_CODE CWfdInterfaceSolution::RefreshDisplayNum(int & nDisplayCount)
{
    nDisplayCount = m_displayCount = 0;
    memset(m_displayHmonitor, 0, sizeof(m_displayHmonitor));
    memset(m_displayRect, 0, sizeof(m_displayRect));

    ::EnumDisplayMonitors(NULL, NULL, CWfdInterfaceSolution::MonitorEnumProc, NULL);
    nDisplayCount = m_displayCount;

    return WFD_ERROR_SUCCESS;
}









// - CWfdInterfaceSolutionSink -


CWfdInterfaceSolutionSink::CWfdInterfaceSolutionSink()
{
    m_pDDSink = NULL;
}

CWfdInterfaceSolutionSink::~CWfdInterfaceSolutionSink()
{
    ReleaseDirectDisplaySourceObject();
}

WFD_ERROR_CODE CWfdInterfaceSolutionSink::CreateDirectDisplaySourceObject()
{
    // Create Direct Display Sink object
    CComPtr<IUnknown> spUnknownSink;
    spUnknownSink.CoCreateInstance(__uuidof(DirectDisplaySink));

    if (!spUnknownSink) {
        return WFD_ERROR_INIT_COM_FAILED;
    } 

    // Get Direct Display Sink interface
    spUnknownSink.QueryInterface(&m_pDDSink);

    if (!m_pDDSink) {
        return WFD_ERROR_INIT_COM_FAILED;
    } 
    // Start listen to events from Direct Display COM server
    m_SinkEventHandler.DispEventAdvise(m_pDDSink);
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdInterfaceSolutionSink::ReleaseDirectDisplaySourceObject()
{
    // Sink cleanup
    m_SinkEventHandler.DispEventUnadvise(m_pDDSink);

    if (m_pDDSink) {
        // Must release the interface before uninitilizing COM.
        // We cannot wait for the member destruction to release the interface.
        m_pDDSink.Release();
        m_pDDSink = NULL;
    }
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdInterfaceSolutionSink::SetConfig(CString & strSourceIpAddress, CString & strUserSpecificName)
{
    if (NULL == m_pDDSink)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }

    // Set Source IP address (Network interface)
    if (strSourceIpAddress.GetLength() <= 0 )
    {
        strSourceIpAddress = _T("127.0.0.1");
    }
    _bstr_t bstrNetworkInterface = strSourceIpAddress;
    m_pDDSink->PutSourceIpAddress(bstrNetworkInterface);


      // Sink session management type
    m_pDDSink->PutSessionManagementType(static_cast<enum tag_dds_session_mgmt_type>(DDS_SESSION_MGMT_RTSP));

    // RTSP port (only used if session type is RTSP)
    m_pDDSink->PutRtspPort(7236);

    // Sink transport type (UDP/RTP)
    m_pDDSink->PutTransportType(static_cast<enum tag_dds_transport_type>(DDS_TRANSPORT_RTP));       

    // RTP port (only used if session type is RTSP)
    m_pDDSink->PutRtpPort(5000);

    // Sink RTSP mode (sink, coupled primary, coupled secondary)
    m_pDDSink->PutRtspMode(static_cast<enum tag_dds_rtsp_mode>(DDS_RTSP_MODE_SINK));


    // Set Config file path
    CString strConfigFilePath;
    // Get application path
    GetModuleFileName(NULL, strConfigFilePath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
    int i = strConfigFilePath.ReverseFind('\\') ; // find first backslash from end
    strConfigFilePath = strConfigFilePath.Left(i + 1) ;
    // Test if "application path\DDServerFiles" exists.
    // If so, use this, otherwise just use the application path
    struct _stat buf;
    if (_tstat(strConfigFilePath + _T("DDServerFiles"), &buf) == 0)
    {
        strConfigFilePath += _T("DDServerFiles");
        strConfigFilePath += "\\";
    }
    _bstr_t bstrConfigFilePath(strConfigFilePath);
    m_pDDSink->PutConfigFilePath(bstrConfigFilePath);

    // UIBC feature enable
    m_pDDSink->PutUIBCFeatureEnable(0);

    // Use external Renderer (only used if session type is RTSP)
    m_pDDSink->PutUseExternalRenderer(1);  

    // Sink User Specific Name
    _bstr_t bstrSpecificName(strUserSpecificName);	
    m_pDDSink->SinkSpecificName(bstrSpecificName);    
    
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdInterfaceSolutionSink::StartSession()
{
    if (NULL == m_pDDSink)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }
    try 
    {
        // Start Session
        m_pDDSink->StartSession();
    }
    catch (_com_error& e) {
        return WFD_ERROR_START_SESSION_FAILED;
    }
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdInterfaceSolutionSink::StopSession()
{
    if (NULL == m_pDDSink)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }
    try 
    {
        // Stop session
        m_pDDSink->StopSession();
    }
    catch (_com_error& e) {
        return WFD_ERROR_STOP_SESSION_FAILED;
    }
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdInterfaceSolutionSink::Play()
{
    if (NULL == m_pDDSink)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }

    try 
    {
        // Play
        m_pDDSink->Play();
    }
    catch (_com_error& e) {
        return WFD_ERROR_PLAY_FAILED;
    }
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdInterfaceSolutionSink::Stop()
{
    if (NULL == m_pDDSink)
    {
        return WFD_ERROR_COM_UNINITIALIZED;
    }

    try 
    {
        // Stop
        m_pDDSink->Stop();
    }
    catch (_com_error e) {
        return WFD_ERROR_PAUSE_FAILED;
    }
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdInterfaceSolutionSink::RegisterCallbacks(EventHandlerProc pEventHandlerProc)
{
    return WFD_ERROR_SUCCESS;
}











// - CWfdWlanSolution - 

CWfdWlanSolution::CWfdWlanSolution(void)
{
	GUID EmptyGUID;
	memset(&EmptyGUID,0,sizeof(GUID));

    m_WifiConnectedGatewayIP.S_un.S_addr = 0;
    m_nWlanScanCondition = 0;
    m_nWlanConnectCondition = 0;
    InitializeCriticalSection(&m_csWlanScanCondition);
    InitializeCriticalSection(&m_csWlanConnectCondition);
    InitializeCriticalSection(&m_csWlanScanResult);
	GuidToString(&EmptyGUID, m_WlanInterfaceGUID);
	GuidToString(&EmptyGUID, m_VirtualAdapterGUID);

}


CWfdWlanSolution::~CWfdWlanSolution(void)
{
    DeleteCriticalSection(&m_csWlanScanCondition);
    DeleteCriticalSection(&m_csWlanConnectCondition);
    DeleteCriticalSection(&m_csWlanScanResult);
}



WFD_ERROR_CODE CWfdWlanSolution::CheckWlanHandle()
{
    DWORD dwError = 0;
	DWORD dwServiceVersion = 0;
    HANDLE hClient = NULL;

    if(NULL == m_hWlanClient)
	{
		if (ERROR_SUCCESS != (dwError = WlanOpenHandle(
				WLAN_API_VERSION,
				NULL,               // reserved
				&dwServiceVersion,
				&hClient
				)))
		{
			return WFD_ERROR_WLAN_ACQUIRE_HANDLE_FAILED;
		}

		// check service version
		if (WLAN_API_VERSION_MAJOR(dwServiceVersion) < WLAN_API_VERSION_MAJOR(WLAN_API_VERSION_2_0))
		{
			WlanCloseHandle(hClient, NULL);
			return WFD_ERROR_WLAN_ACQUIRE_HANDLE_FAILED;
		}
		m_hWlanClient = hClient;
	}
    return WFD_ERROR_SUCCESS;
}

/*
int CWlanSolution::UpdateWirelessInterfaceInfo()
{
    m_vecWirelessInterfaceInfo.clear();
    WIRELESS_INTERFACE_INFO tempNode;
    size_t nConvertSize = 0;
    wchar_t wbuf[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
    char buf[100] = {0};   


    // Get or refresh Wlan Interface
    // Note : the hostednetwork can't be retrive

    // WlanEnumInterfaces
    DWORD dwError = 0;
	PWLAN_INTERFACE_INFO_LIST pIntfList = NULL;

	if (ERROR_SUCCESS != (dwError = WlanEnumInterfaces(
		m_hWlanClient,
		NULL,               // reserved
		&pIntfList
		)))
	{
		return 1;
	}

    // GetAdapterInfo
    ULONG ulMemSize = 0;
    PIP_ADAPTER_INFO pIpAdapterInfo = NULL; 
    PIP_ADAPTER_INFO pIpAdapterInfoCur = NULL;	
    char *pMem = NULL;

    GetAdaptersInfo(NULL, &ulMemSize);
    pMem = new char[ulMemSize];
    pIpAdapterInfo = (PIP_ADAPTER_INFO)pMem;
    if (ERROR_SUCCESS == GetAdaptersInfo(pIpAdapterInfo, &ulMemSize))
    {
        in_addr hosted_network_addr;
        hosted_network_addr.S_un.S_addr = 0;
        for (pIpAdapterInfoCur = pIpAdapterInfo; NULL != pIpAdapterInfoCur; pIpAdapterInfoCur = pIpAdapterInfoCur->Next)
        {
            // check if this adapter info is hosted network
            // Note: the interface info list retrived by WlanEnum not include Hostednetwork
            // so if IF_TYPE_IEEE80211 == pIpAdapterInfoCur->Type, and is not included in m_vectorWlanInterfaceSink, means hostednetwork
            // Note : the interface desciption in this structure use char/wchar_t date type explicitly,
            // so we should handle this by transfering encoding instead of using _t series macros
            if(IF_TYPE_IEEE80211 == pIpAdapterInfoCur->Type)
            {
                ULONG i = 0;
                for (; i < pIntfList->dwNumberOfItems; i++)
                {
                    memset(wbuf, 0, sizeof(wbuf));
                    MultiByteToWideChar(CP_ACP,
                        0,
                        pIpAdapterInfoCur->Description,
                        sizeof(pIpAdapterInfoCur->Description),
                        wbuf,
                        MAX_ADAPTER_DESCRIPTION_LENGTH + 4);

                    if (0 == wcscmp(wbuf, pIntfList->InterfaceInfo[i].strInterfaceDescription))
                    {                        
                        tempNode.cIsVirtualInterface = 0;
                        tempNode.wlanInterfaceInfo = pIntfList->InterfaceInfo[i];
                        tempNode.localIp.S_un.S_addr = inet_addr(pIpAdapterInfoCur->IpAddressList.IpAddress.String);
                        tempNode.gatewayIp.S_un.S_addr = inet_addr(pIpAdapterInfoCur->GatewayList.IpAddress.String);
                        if ((tempNode.localIp.S_un.S_addr != 0) && (tempNode.localIp.S_un.S_addr != 0xFFFFFFFF))
                        {
                            tempNode.cIsConnected = 1;
                        }
                        else
                        {
                            tempNode.cIsConnected = 0;
                        }
                        m_vecWirelessInterfaceInfo.push_back(tempNode);
                        break;
                    }
                }
                // hosted network
                if (i == pIntfList->dwNumberOfItems)
                {
                    tempNode.cIsVirtualInterface = 1;
                    tempNode.localIp.S_un.S_addr = inet_addr(pIpAdapterInfoCur->IpAddressList.IpAddress.String);
                    tempNode.gatewayIp.S_un.S_addr = inet_addr(pIpAdapterInfoCur->GatewayList.IpAddress.String);

                    PWLAN_HOSTED_NETWORK_STATUS wlanSoftAPStatus = NULL;
                    WlanHostedNetworkQueryStatus(m_hWlanClient, &wlanSoftAPStatus ,NULL);
                    tempNode.hostedNetworkState = wlanSoftAPStatus->HostedNetworkState;
                    WlanFreeMemory(wlanSoftAPStatus);
                    wlanSoftAPStatus = NULL;

                    tempNode.cIsConnected = 0;
                    if (wlan_hosted_network_active == tempNode.hostedNetworkState)
                    {
                        if(tempNode.gatewayIp.S_un.S_un_b.s_b1 == 169 && tempNode.gatewayIp.S_un.S_un_b.s_b2 == 254)
                        {
                            tempNode.cIsConnected = 0;
                        }
                        else
                        {
                            tempNode.cIsConnected = 1;
                        }
                    }
                    m_vecWirelessInterfaceInfo.push_back(tempNode);
                } // if(i == pIntfList->dwNumberOfItems)
            }// if(IF_TYPE_IEEE80211 == pIpAdapterInfoCur->Type)
        } // for (pIpAdapterInfoCur = pIpAdapterInfo; NULL != pIpAdapterInfoCur; pIpAdapterInfoCur = pIpAdapterInfoCur->Next)
    }
    else
    {
        tempNode.cIsVirtualInterface = 0;
        if (pIntfList->dwNumberOfItems > 0)
	    {
		    for (ULONG i = 0; i < pIntfList->dwNumberOfItems; i++)
		    {
			    m_vectorWlanInterfaceSink.push_back(pIntfList->InterfaceInfo[i]);
                tempNode.wlanInterfaceInfo = pIntfList->InterfaceInfo[i];
                m_vecWirelessInterfaceInfo.push_back(tempNode);
		    }
	    }
    }

    delete [] pMem;

	if (pIntfList != NULL)
	{
		WlanFreeMemory(pIntfList);
		pIntfList = NULL;
	}   
    
    return 0;
}


int CWlanSolution::RefreshWlanInterfaceArray()
{
    // Get or refresh Wlan Interface
    // Note : the hostednetwork can't be retrive
    DWORD dwError = 0;
	PWLAN_INTERFACE_INFO_LIST pIntfList = NULL;
	m_vectorWlanInterfaceSink.clear();
	if (ERROR_SUCCESS != (dwError = WlanEnumInterfaces(
		m_hWlanClient,
		NULL,               // reserved
		&pIntfList
		)))
	{
		return 1;
	}
	
	if (pIntfList->dwNumberOfItems > 0)
	{
		for (ULONG i = 0; i < pIntfList->dwNumberOfItems; i++)
		{
			m_vectorWlanInterfaceSink.push_back(pIntfList->InterfaceInfo[i]);
		}
	}

	if (pIntfList != NULL)
	{
		WlanFreeMemory(pIntfList);
		pIntfList = NULL;
	}
    return 0;
}
*/

WFD_ERROR_CODE CWfdWlanSolution::RegisterNotificationFunc()
{
    DWORD dwPrevNotifType;
    DWORD dwError;
	if(ERROR_SUCCESS != (dwError = WlanRegisterNotification(
					m_hWlanClient,
					WLAN_NOTIFICATION_SOURCE_ALL, //WLAN_NOTIFICATION_SOURCE_ACM | WLAN_NOTIFICATION_SOURCE_MSM,
					FALSE,			// do not ignore duplications
					(WLAN_NOTIFICATION_CALLBACK)CWfdWlanSolution::NotificationCallback,
					(PVOID)this,
					NULL,           // reserved
					&dwPrevNotifType
					)))
	{
		return WFD_ERROR_WLAN_REGISTER_MESSAGE_HANDLER_FAILED;
	}   
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdWlanSolution::UnregisterNotificationFunc()
{
    DWORD dwPrevNotifType;
    DWORD dwError;
	if(ERROR_SUCCESS != (dwError = WlanRegisterNotification(
					m_hWlanClient,
					WLAN_NOTIFICATION_SOURCE_NONE, //WLAN_NOTIFICATION_SOURCE_ACM | WLAN_NOTIFICATION_SOURCE_MSM,
					FALSE,			// do not ignore duplications
					NULL,
					(PVOID)this,
					NULL,           // reserved
					&dwPrevNotifType
					)))
	{
		return WFD_ERROR_WLAN_REGISTER_MESSAGE_HANDLER_FAILED;
	}   
    return WFD_ERROR_SUCCESS;
}


VOID CWfdWlanSolution::NotificationCallback(
		__in PWLAN_NOTIFICATION_DATA pNotifData, 
		__in_opt PVOID pContext
		)
{
    PWLAN_CONNECTION_NOTIFICATION_DATA pConnNotifData = NULL;
    PWLAN_MSM_NOTIFICATION_DATA pMsmNotifData = NULL;
	PWLAN_HOSTED_NETWORK_DATA_PEER_STATE_CHANGE pHstNotifData = NULL;

	CString s;
	s.Format(_T("wlan cmd: src: %x, type:%x"), pNotifData->NotificationSource, pNotifData->NotificationCode);
	// AfxMessageBox(s);
    // CWlanSolution * pWlanConnectionInst = (CWlanSolution *)pContext;
	if (WLAN_NOTIFICATION_SOURCE_ACM == pNotifData->NotificationSource)
	{
		switch (pNotifData->NotificationCode)
		{
		case wlan_notification_acm_disconnected:
			{
				EnterCriticalSection(&m_PeerChangeCond);
				wlanEvtType = WLAN_EVT_STA_DISCONNECT;
				LeaveCriticalSection(&m_PeerChangeCond);
				SetEvent(hPeerChangeEvent);
			}
			break;

		}
	}
	if (WLAN_NOTIFICATION_SOURCE_HNWK == pNotifData->NotificationSource)
	{
		switch (pNotifData->NotificationCode)
		{
		case wlan_hosted_network_peer_state_change:
			{
				pHstNotifData = (PWLAN_HOSTED_NETWORK_DATA_PEER_STATE_CHANGE)pNotifData->pData;
				switch (pHstNotifData->PeerStateChangeReason)
				{
				case wlan_hosted_network_reason_peer_arrived:
					rsnMac.Format(_T("Connect client mac: %02x:%02x:%02x:%02x:%02x; %02x"), 
						pHstNotifData->NewState.PeerMacAddress[0],
						pHstNotifData->NewState.PeerMacAddress[1],
						pHstNotifData->NewState.PeerMacAddress[2],
						pHstNotifData->NewState.PeerMacAddress[3],
						pHstNotifData->NewState.PeerMacAddress[4],
						pHstNotifData->NewState.PeerMacAddress[5]);
					// AfxMessageBox(rsnMac);
					break;
				case wlan_hosted_network_reason_peer_departed:
					    EnterCriticalSection(&m_PeerChangeCond);
						rsnMac.Format(_T("%02x:%02x:%02x:%02x:%02x:%02x"), 
							pHstNotifData->NewState.PeerMacAddress[0],
							pHstNotifData->NewState.PeerMacAddress[1],
							pHstNotifData->NewState.PeerMacAddress[2],
							pHstNotifData->NewState.PeerMacAddress[3],
							pHstNotifData->NewState.PeerMacAddress[4],
							pHstNotifData->NewState.PeerMacAddress[5]);
						wlanEvtType = WLAN_EVT_AP_PEER_DEPARTED;
						LeaveCriticalSection(&m_PeerChangeCond);
						SetEvent(hPeerChangeEvent);
					break;
				}
			}
			break;
		default:
			break;
		}
	}

    switch(pNotifData->NotificationCode)
    {
    case wlan_notification_acm_connection_complete:
        if (pNotifData->dwDataSize < sizeof(WLAN_CONNECTION_NOTIFICATION_DATA))
		{
			break;
		}
        pConnNotifData = (PWLAN_CONNECTION_NOTIFICATION_DATA)pNotifData->pData;
        if ( WLAN_REASON_CODE_SUCCESS == pConnNotifData->wlanReasonCode )
        {
             CWfdWlanSolution::Instance()->SetConnectConditionCriticalSectionValue(1);
        }
        break;
    case wlan_notification_acm_connection_start:
        CWfdWlanSolution::Instance()->SetConnectConditionCriticalSectionValue(0);
        break;
    case wlan_notification_acm_scan_complete:
        CWfdWlanSolution::Instance()->SetScanConditionCriticalSectionValue(1); 
        CWfdWlanSolution::Instance()->SetScanResultCriticalSectionValue(0);
        break;
    case wlan_notification_acm_scan_fail:
        CWfdWlanSolution::Instance()->SetScanConditionCriticalSectionValue(1); 
        CWfdWlanSolution::Instance()->SetScanResultCriticalSectionValue(1);
        break;
		/*
    case wlan_notification_acm_scan_list_refresh:
        break;
		*/
    case wlan_notification_msm_roaming_start:
        pMsmNotifData = (PWLAN_MSM_NOTIFICATION_DATA)pNotifData->pData;
        break;
    default:
        break;
    }
}

// - hosted network -

WFD_ERROR_CODE CWfdWlanSolution::HostedNetworkQueryState(int & state)
{
    state = -1;
    PWLAN_HOSTED_NETWORK_STATUS wlanSoftAPStatus = NULL;
    DWORD dwRet = WlanHostedNetworkQueryStatus(m_hWlanClient, &wlanSoftAPStatus, NULL);
    if (ERROR_SUCCESS != dwRet)
    {
        return WFD_ERROR_HOSTEDNETWORK_QUERY_STATE_FAILED;
    }

    state = wlanSoftAPStatus->HostedNetworkState;
    WlanFreeMemory(wlanSoftAPStatus);
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdWlanSolution::HostedNetworkSetMode(BOOL bIsAllow)
{
    WLAN_HOSTED_NETWORK_REASON dwFailedReason;
    DWORD dwReturnValue = WlanHostedNetworkSetProperty(m_hWlanClient, 
        wlan_hosted_network_opcode_enable, 
        sizeof(BOOL), 
        &bIsAllow, 
        &dwFailedReason, 
        NULL);

    if(ERROR_SUCCESS != dwReturnValue)
    {
        return WFD_ERROR_HOSTEDNETWORK_SET_PROPERTY_FAILED;
    }
    return WFD_ERROR_SUCCESS;
}


WFD_ERROR_CODE CWfdWlanSolution::HostedNetworkUpdateIp(in_addr & hostednetworkip)
{
    hostednetworkip.S_un.S_addr = 0;

    int state;
    if (WFD_ERROR_SUCCESS != HostedNetworkQueryState(state) 
        || wlan_hosted_network_active != state)
    {        
        return WFD_ERROR_HOSTEDNETWORK_INVALID_STATUS;
    }

    size_t nConvertSize = 0;
    wchar_t wbuf[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];

    // WlanEnumInterfaces
    DWORD dwError = 0;
	PWLAN_INTERFACE_INFO_LIST pIntfList = NULL;

	if (ERROR_SUCCESS != (dwError = WlanEnumInterfaces(
		m_hWlanClient,
		NULL,               // reserved
		&pIntfList
		)))
	{
		return WFD_ERROR_HOSTEDNETWORK_GET_IP_FAILED;
	}

    // GetAdapterInfo
    ULONG ulMemSize = 0;
    PIP_ADAPTER_INFO pIpAdapterInfo = NULL; 
    PIP_ADAPTER_INFO pIpAdapterInfoCur = NULL;	
    char *pMem = NULL;

    GetAdaptersInfo(NULL, &ulMemSize);
    pMem = new char[ulMemSize];
    pIpAdapterInfo = (PIP_ADAPTER_INFO)pMem;
    GetAdaptersInfo(pIpAdapterInfo, &ulMemSize);
    pIpAdapterInfoCur = pIpAdapterInfo;

    in_addr hosted_network_addr;
    hosted_network_addr.S_un.S_addr = 0;
    for (; NULL != pIpAdapterInfoCur; pIpAdapterInfoCur = pIpAdapterInfoCur->Next)
    {
        // check if this adapter info is hosted network
        // Note: the interface info list retrived by WlanEnum not include Hostednetwork
        // so the one whose IF_TYPE_IEEE80211 == pIpAdapterInfoCur->Type, and which is not exist in pIntfList, is hostednetwork
        // Note : the interface desciption in this structure use char/wchar_t date type explicitly,
        // so we should handle this by transfering encoding instead of using _t series macros

		//Jim: Bug - besides the hostednetwork, the "Wi-Fi Direct Virtual ... adatper" may be in pIpAdapterInfoCur and it has no IP
        if (IF_TYPE_IEEE80211 == pIpAdapterInfoCur->Type)
        {
            ULONG i = 0;
            for (; i < pIntfList->dwNumberOfItems; i++)
            {
                memset(wbuf, 0, sizeof(wbuf));
                MultiByteToWideChar(CP_ACP,
                    0,
                    pIpAdapterInfoCur->Description,
                    sizeof(pIpAdapterInfoCur->Description),
                    wbuf,
                    MAX_ADAPTER_DESCRIPTION_LENGTH + 4);

                if (0 == wcscmp(wbuf, pIntfList->InterfaceInfo[i].strInterfaceDescription))
                {
                    break;
                }
            }
            // hosted network
            if (i == pIntfList->dwNumberOfItems)
            {
                hosted_network_addr.S_un.S_addr = inet_addr(pIpAdapterInfoCur->IpAddressList.IpAddress.String);                    
                if(hosted_network_addr.S_un.S_un_b.s_b1 == 169 && hosted_network_addr.S_un.S_un_b.s_b2 == 254)
                {
                    hosted_network_addr.S_un.S_addr = 0;
                }
                break;
            } // if(i == pIntfList->dwNumberOfItems)
        }
    }
    delete [] pMem;

    hostednetworkip = hosted_network_addr;
    if(0 == hosted_network_addr.S_un.S_addr)
    {
        return WFD_ERROR_HOSTEDNETWORK_GET_IP_FAILED;
    }
    return WFD_ERROR_SUCCESS;
}

// Note : return 2 means the NIC is not in use
// if NIC is not in use,soft ap param can be set successfully, but can't be started.
WFD_ERROR_CODE CWfdWlanSolution::HostedNetworkStartSoftAp(IN CString strSSID, IN CString strSecondaryKey, OUT in_addr& ip_local_address)
{    
    WLAN_HOSTED_NETWORK_REASON dwFailedReason;
	DWORD dwReturnValue;

    // Step 1 : Check all pre-conditions of starting hosted network
	if(WFD_ERROR_WLAN_ACQUIRE_HANDLE_FAILED == CheckWlanHandle())
    {
        return WFD_ERROR_HOSTEDNETWORK_START_FAILED;
    }

    // Step 2 : make sure hostednetwork is in correct status (wlan_hosted_network_idle)
    int state;
    if (WFD_ERROR_SUCCESS != HostedNetworkQueryState(state))
    {
        return WFD_ERROR_HOSTEDNETWORK_START_FAILED;
    }

    // the resource is not in correct state
    if (-1 == state)
    {
        return WFD_ERROR_HOSTEDNETWORK_INVALID_STATUS;
    }
    // hosted network mode is disallow
    else if(wlan_hosted_network_unavailable == state)
    {
        if (WFD_ERROR_SUCCESS != HostedNetworkSetMode(TRUE))
        {
            HostedNetworkSetMode(FALSE);
            return WFD_ERROR_HOSTEDNETWORK_ALLOW_FAILED;
        }
    }
	if (WFD_ERROR_WLAN_REGISTER_MESSAGE_HANDLER_FAILED == RegisterNotificationFunc())
    {
        return WFD_ERROR_WLAN_SCAN_FAILED;
    }
    else
    {
    }

    // Step 3 : Get specific SSID("Qualcomm_Atheros_Miracast" by default)
    if(strSSID.GetLength() <= 0)
    {
        strSSID = _T("Qualcomm_Atheros_Miracast");
    }

    // Step 4 : Get secondary key("1234567890" by default)
    if ( strSecondaryKey.GetLength() < 8 || strSecondaryKey.GetLength() > 63 )
    {
        strSecondaryKey = _T("1234567890");
    }


    // Step 5 : Set hosted network connection setting - SSID(ANSI char), Max number of peer(100)
    // Equal to command line "netsh wlan set hostednetwork ssid=xxxxx"
    WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS wlanConnectionSetting;
    memset(&wlanConnectionSetting, 0, sizeof(WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS));

#ifdef _UNICODE
    WideCharToMultiByte(CP_ACP,
        0,
        strSSID.GetBuffer(0),
        strSSID.GetLength(),
        (LPSTR)wlanConnectionSetting.hostedNetworkSSID.ucSSID,
        32,
        NULL,
        NULL);
#else
    memcpy(wlanConnectionSetting.hostedNetworkSSID.ucSSID, strSSID.GetBuffer(0), strlen(strSSID.GetBuffer(0)));
#endif   

    wlanConnectionSetting.hostedNetworkSSID.uSSIDLength = strlen((const char*)wlanConnectionSetting.hostedNetworkSSID.ucSSID);

    wlanConnectionSetting.dwMaxNumberOfPeers = 100;	

    dwReturnValue = WlanHostedNetworkSetProperty(m_hWlanClient, 
        wlan_hosted_network_opcode_connection_settings, 
        sizeof(WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS), 
        &wlanConnectionSetting,
        &dwFailedReason,
        NULL);
    if(ERROR_SUCCESS != dwReturnValue)
    {
        return WFD_ERROR_HOSTEDNETWORK_START_FAILED;
    }


    // Step 6 : Set hosted network secondary key(ANSI char)
    // Equal to command line "netsh wlan set hostednetwork key=xxxxx"
    UCHAR keyBuf[100] = {0};
#ifdef _UNICODE
    WideCharToMultiByte(CP_ACP,
        0,
        strSecondaryKey.GetBuffer(0),
        strSecondaryKey.GetLength(),
        (LPSTR)keyBuf,
        100,
        NULL,
        NULL);
#else
    memcpy(keyBuf, strSecondaryKey.GetBuffer(0), strlen(strSecondaryKey.GetBuffer(0)));
#endif
    dwReturnValue = WlanHostedNetworkSetSecondaryKey(m_hWlanClient,
        strlen((const char*)keyBuf) + 1,
        keyBuf,
        TRUE,
        FALSE,
        &dwFailedReason,
        NULL);
    if(ERROR_SUCCESS != dwReturnValue)
    {
        return WFD_ERROR_HOSTEDNETWORK_START_FAILED;
    }

    // Step 7 : Start hosted network
    // Equal to command line "netsh wlan start hostednetwork"
    dwReturnValue = WlanHostedNetworkStartUsing(m_hWlanClient, &dwFailedReason, NULL);    
    if(ERROR_SUCCESS != dwReturnValue)
    {
        if (wlan_hosted_network_reason_interface_unavailable == dwFailedReason)
        {
            return WFD_ERROR_HOSTEDNETWORK_NIC_NOT_ENABLE;
        }
        return WFD_ERROR_HOSTEDNETWORK_START_FAILED;
    }

    // wait until getting correct ip address
	// Jim: bug - HostedNetworkUpdateIp has bug to identify the hostednetwork corrently
 /*   unsigned int nWhileCount = 0;
    in_addr hostednetworkip;
    while(true)
    {
        nWhileCount ++;
        // Timeout is 4s(20 * 200 milliseconds)
        if(nWhileCount > 100)
        {
            HostedNetworkStopSoftAp();
            return WFD_ERROR_HOSTEDNETWORK_START_FAILED;
        }

        if (WFD_ERROR_SUCCESS != HostedNetworkUpdateIp(hostednetworkip))
        {
            Sleep(200);
        }
        else
        {
            break;
        }
    }
	
    ip_local_address = hostednetworkip;*/
    SetAtherosNICInfomationElement(true);
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdWlanSolution::HostedNetworkStopSoftAp()
{
	UnregisterNotificationFunc();
    SetAtherosNICInfomationElement(false);
    WLAN_HOSTED_NETWORK_REASON dwFailedReason;
	DWORD dwReturnValue;
    if (WFD_ERROR_WLAN_ACQUIRE_HANDLE_FAILED == CheckWlanHandle())
    {
        return WFD_ERROR_HOSTEDNETWORK_STOP_FAILED;
    }

    dwReturnValue = WlanHostedNetworkForceStop(m_hWlanClient, &dwFailedReason, NULL);
    if (ERROR_SUCCESS != dwReturnValue)
    {
        return WFD_ERROR_HOSTEDNETWORK_STOP_FAILED;
    }

    return WFD_ERROR_SUCCESS;
}




// - scan -

int CWfdWlanSolution::SetScanConditionCriticalSectionValue(unsigned int nWlanScanCondition)
{
    EnterCriticalSection(&m_csWlanScanCondition);
    m_nWlanScanCondition = nWlanScanCondition;
    LeaveCriticalSection(&m_csWlanScanCondition);
    return 0;
}

unsigned int CWfdWlanSolution::GetScanConditionCriticalSectionValue()
{
    unsigned int nRet = 0;
    EnterCriticalSection(&m_csWlanScanCondition);
    nRet = m_nWlanScanCondition;
    LeaveCriticalSection(&m_csWlanScanCondition);
    return nRet;
}

int CWfdWlanSolution::SetScanResultCriticalSectionValue(unsigned int nWlanScanResult)
{
    EnterCriticalSection(&m_csWlanScanResult);
    m_nWlanScanResult = nWlanScanResult;
    LeaveCriticalSection(&m_csWlanScanResult);
    return 0;
}

unsigned int CWfdWlanSolution::GetScanResultCriticalSectionValue()
{
    unsigned int nRet = 0;
    EnterCriticalSection(&m_csWlanScanResult);
    nRet = m_nWlanScanResult;
    LeaveCriticalSection(&m_csWlanScanResult);
    return nRet;
}

WFD_ERROR_CODE CWfdWlanSolution::EnumWlanAdapterInterfaceGUID()
{
	DWORD dwError = 0;
    PWLAN_INTERFACE_INFO_LIST pIntfList = NULL;
	if(WFD_ERROR_WLAN_ACQUIRE_HANDLE_FAILED == CheckWlanHandle())
    {
        return WFD_ERROR_HOSTEDNETWORK_START_FAILED;
    }
    if (ERROR_SUCCESS != (dwError = WlanEnumInterfaces(
        m_hWlanClient,
        NULL,               // reserved
        &pIntfList
        )))
    {
        return WFD_ERROR_OTHER;
    }

    if (pIntfList->dwNumberOfItems == 0)
    {
        return WFD_ERROR_OTHER;
    }

    
  //only considering the case that only one Wlan adapter presents
    GuidToString(&pIntfList->InterfaceInfo[0].InterfaceGuid,  m_WlanInterfaceGUID);      

    if (pIntfList != NULL)
    {
        WlanFreeMemory(pIntfList);
        pIntfList = NULL;
    }    
    return WFD_ERROR_SUCCESS;
}


WFD_ERROR_CODE CWfdWlanSolution::EnumVirtualAdapterInterfaceGUID()
{
	GUID EmptyGUID;
	WFD_ERROR_CODE ret;
	memset(&EmptyGUID,0,sizeof(GUID));

	//Step 1: Get the Wlan adapter's GUID first
	if( IsSameGUID(m_WlanInterfaceGUID, &EmptyGUID) )
	{
		if((ret=EnumWlanAdapterInterfaceGUID()) != WFD_ERROR_SUCCESS)
			return ret;
	}

	//Step 2: Find the adapter
	// 1) 80211 type
	// 2) GUID does not equal to m_WlanInterfaceGUID
        
	// GetAdapterInfo

    ULONG ulMemSize = 0;
    PIP_ADAPTER_INFO pIpAdapterInfo = NULL; 
    PIP_ADAPTER_INFO pIpAdapterInfoCur = NULL;	
    char *pMem = NULL;
	CString s;
	wchar_t wbuf[264] = {0};

    GetAdaptersInfo(NULL, &ulMemSize);
    pMem = new char[ulMemSize];
    pIpAdapterInfo = (PIP_ADAPTER_INFO)pMem;
    if (!ERROR_SUCCESS == GetAdaptersInfo(pIpAdapterInfo, &ulMemSize))
	{
		return WFD_ERROR_OTHER;
	}
    pIpAdapterInfoCur = pIpAdapterInfo;

	for (; NULL != pIpAdapterInfoCur; pIpAdapterInfoCur = pIpAdapterInfoCur->Next)
	{
		//in case only 1 Wi-Fi adapter
		if (IF_TYPE_IEEE80211 == pIpAdapterInfoCur->Type 
			&& memcmp(pIpAdapterInfoCur->AdapterName, m_WlanInterfaceGUID,GUID_STR_LEN)
			&& !strstr(pIpAdapterInfoCur->Description, "Direct Virtual Adapter"))
		{
			MultiByteToWideChar(CP_ACP,
				0,
				(char *)(pIpAdapterInfoCur->Description),
				132,
				wbuf,
				264);
			s.Format(_T("card:%s, mac: %02x: %02x: %02x: %02x: %02x: %02x"), wbuf, pIpAdapterInfoCur->Address[0],
				pIpAdapterInfoCur->Address[1],
				pIpAdapterInfoCur->Address[2],
				pIpAdapterInfoCur->Address[3],
				pIpAdapterInfoCur->Address[4],
				pIpAdapterInfoCur->Address[5]);
			// AfxMessageBox(s);
			memcpy(m_VirtualAdapterGUID,  pIpAdapterInfoCur->AdapterName,GUID_STR_LEN);
			//return WFD_ERROR_SUCCESS;
		}
	}    


    return WFD_ERROR_OTHER;
}

WFD_ERROR_CODE CWfdWlanSolution::ChooseUsedInterface()
{
    // Get or refresh Wlan Interface
    // Note : the hostednetwork can't be retrive
    DWORD dwError = 0;
    PWLAN_INTERFACE_INFO_LIST pIntfList = NULL;
    if (ERROR_SUCCESS != (dwError = WlanEnumInterfaces(
        m_hWlanClient,
        NULL,               // reserved
        &pIntfList
        )))
    {
        return WFD_ERROR_OTHER;
    }

    if (pIntfList->dwNumberOfItems == 0)
    {
        return WFD_ERROR_WLAN_NO_VALID_INTERFACE;
    }

    bool bIsChosed = false;
    for (ULONG i = 0; i < pIntfList->dwNumberOfItems; i++)
    {
        if (wlan_interface_state_not_ready != pIntfList->InterfaceInfo[i].isState)
        {
            m_usedWlanInterface = pIntfList->InterfaceInfo[i];
            bIsChosed = true;
            break;
        }        
    }

    if (false == bIsChosed)
    {
        return WFD_ERROR_WLAN_NO_VALID_INTERFACE;
    }

    if (pIntfList != NULL)
    {
        WlanFreeMemory(pIntfList);
        pIntfList = NULL;
    }    
    return WFD_ERROR_SUCCESS;
}


BYTE* CWfdWlanSolution::getWifiCardMacAddr(BYTE * VirtualAdapterAddress)
{
	//Considering the virtual adapter may not be present if never set hostednetwork mode=allow
	//Enum the Wlan Adapter with WlanEnumInterfaces, it will not enum Virtual Adapter
	//Then match the GUID to find the WLan adapter's MAC
	//Caculate the Virtual Adapater's MAC by Wlan adpater's MAC + 1

	//Step 1, Enum the Wlan Adapter with WlanEnumInterfaces
	GUID EmptyGUID;
	memset(&EmptyGUID,0,sizeof(GUID));
	if( IsSameGUID(m_WlanInterfaceGUID, &EmptyGUID) )
	{
		if(EnumWlanAdapterInterfaceGUID() != WFD_ERROR_SUCCESS)
		{
			VirtualAdapterAddress=NULL;
			return NULL;
		}
	}

 // GetAdapterInfo
    ULONG ulMemSize = 0;
    PIP_ADAPTER_INFO pIpAdapterInfo = NULL; 
    PIP_ADAPTER_INFO pIpAdapterInfoCur = NULL;	
    char *pMem = NULL;

    GetAdaptersInfo(NULL, &ulMemSize);
    pMem = new char[ulMemSize];
    pIpAdapterInfo = (PIP_ADAPTER_INFO)pMem;
    if (!ERROR_SUCCESS == GetAdaptersInfo(pIpAdapterInfo, &ulMemSize))
	{
		VirtualAdapterAddress=NULL;
		return NULL;
	}
    pIpAdapterInfoCur = pIpAdapterInfo;

	for (; NULL != pIpAdapterInfoCur; pIpAdapterInfoCur = pIpAdapterInfoCur->Next)
	{
		if (IF_TYPE_IEEE80211 == pIpAdapterInfoCur->Type && !memcmp(pIpAdapterInfoCur->AdapterName, m_WlanInterfaceGUID,GUID_STR_LEN))
		{
			//Caculate the Virtual Adapater's MAC by Wlan adpater's MAC + 1
			//because the Virtual Adapter may not be created so impossible to enum it
			memcpy(VirtualAdapterAddress,pIpAdapterInfoCur->Address,6);
			VirtualAdapterAddress[6-1]+=1;
			return VirtualAdapterAddress;
		}

		if (IF_TYPE_IEEE80211 == pIpAdapterInfoCur->Type && !memcmp(pIpAdapterInfoCur->AdapterName, m_VirtualAdapterGUID,GUID_STR_LEN))
		{
			
			//Virtual Adapter found			
			memcpy(VirtualAdapterAddress,pIpAdapterInfoCur->Address,6);
			
			return VirtualAdapterAddress;
		}
	}
	VirtualAdapterAddress = NULL;
	return NULL;
}

bool CWfdWlanSolution::getWifiNames(char* pStrWlanAdatperName, char* pStrHostNetworkAdapterName)
{
 return true;
}

// pendding function, should be run in another thread
WFD_ERROR_CODE CWfdWlanSolution::ScanBssList(std::vector<WLAN_BSS_ENTRY> & vecSelectedBSSEntry, bool bZero,int matchType)
{    
    if (WFD_ERROR_WLAN_ACQUIRE_HANDLE_FAILED == CheckWlanHandle())
    {
        return WFD_ERROR_WLAN_SCAN_FAILED;
    }

    int nRet = ChooseUsedInterface();
    if (WFD_ERROR_WLAN_NO_VALID_INTERFACE == nRet)
    {        
        return WFD_ERROR_WLAN_NO_VALID_INTERFACE;
    }
    else if(WFD_ERROR_OTHER == nRet)
    {
        return WFD_ERROR_WLAN_SCAN_FAILED;
    }

    //if (WFD_ERROR_WLAN_REGISTER_MESSAGE_HANDLER_FAILED == RegisterNotificationFunc())
    {
    //    return WFD_ERROR_WLAN_SCAN_FAILED;
    }

    if (ERROR_SUCCESS != WlanScan(m_hWlanClient, &m_usedWlanInterface.InterfaceGuid, NULL, NULL, NULL))
    {
        return WFD_ERROR_WLAN_SCAN_FAILED;
    }    

    // wait for scan finished
    SetScanConditionCriticalSectionValue(0);
    unsigned int nCondition = 0;
    unsigned int nWhileCount = 0;
    while(true)
    {
        nWhileCount ++;
        // Timeout is 8s(40 * 200 milliseconds)
        if(nWhileCount > 40)
        {
            nCondition = 0;
            break;
        }        
        
        nCondition = GetScanConditionCriticalSectionValue();
        if(0 == nCondition)            
        {
            Sleep(200);
        }
        else
        {
            break;
        }
    }
    if (0 == nCondition)
    {
        return WFD_ERROR_WLAN_SCAN_TIMEOUT;
    }
    if (1 == GetScanResultCriticalSectionValue())
    {
        return WFD_ERROR_WLAN_SCAN_FAILED;
    }

    vecSelectedBSSEntry.clear();
    if (bZero)
    {
        m_vectorSelectedBSSEntry.clear();
    }

    DWORD dwError;
    PWLAN_BSS_LIST pWlanBssList = NULL;

    dwError = WlanGetNetworkBssList(
        m_hWlanClient,
        &m_usedWlanInterface.InterfaceGuid,
        NULL,
        dot11_BSS_type_any,
        FALSE,
        NULL,
        &pWlanBssList
        );
    if ( ERROR_SUCCESS != dwError )
    {
        return WFD_ERROR_WLAN_GET_BSS_LIST_FAILED;
    }

    PBYTE pByteIE;
    for (DWORD i = 0; i < pWlanBssList->dwNumberOfItems; i++)
    {	
        PWLAN_BSS_ENTRY pBssEntry = &pWlanBssList->wlanBssEntries[i];

        //Match in SSID
        if(matchType)
		{
			char prefix[13];
			strncpy_s(prefix,(char*)pBssEntry->dot11Ssid.ucSSID,12);
			prefix[12]='\0';
			if(0 == strcmp(prefix, SMARTSHARING_SSID_PREFIX))
			{
				m_vectorSelectedBSSEntry.push_back(pWlanBssList->wlanBssEntries[i]);
                vecSelectedBSSEntry.push_back(pWlanBssList->wlanBssEntries[i]);
			}
		}else{//Match in the Vender Specific IE .dot11Ssid.ucSSID
			pByteIE = (PBYTE)pBssEntry + pBssEntry->ulIeOffset;

            ULONG ulIESize = pBssEntry->ulIeSize - sizeof(QUALCOMM_WIFI_DISPLAY_ANSI_IDENTIFICATION);
            for(unsigned int j = 0; j < ulIESize; j++)
            {
                if(0 == strcmp(((const char*)pByteIE) + j, QUALCOMM_WIFI_DISPLAY_ANSI_IDENTIFICATION))
                {
                    m_vectorSelectedBSSEntry.push_back(pWlanBssList->wlanBssEntries[i]);
                    vecSelectedBSSEntry.push_back(pWlanBssList->wlanBssEntries[i]);
                    break;
                }
            }
		}
        
    }

    if(pWlanBssList)
    {
        WlanFreeMemory(pWlanBssList);
        pWlanBssList = NULL;
    }

    return WFD_ERROR_SUCCESS;
}






// - connect - 
int CWfdWlanSolution::SetConnectConditionCriticalSectionValue(unsigned int nWlanScanCondition)
{
    EnterCriticalSection(&m_csWlanConnectCondition);
    m_nWlanConnectCondition = nWlanScanCondition;
    LeaveCriticalSection(&m_csWlanConnectCondition);
    return 0;
}

unsigned int CWfdWlanSolution::GetConnectConditionCriticalSectionValue()
{
    unsigned int nRet = 0;
    EnterCriticalSection(&m_csWlanConnectCondition);
    nRet = m_nWlanConnectCondition;
    LeaveCriticalSection(&m_csWlanConnectCondition);
    return nRet;
}

WFD_ERROR_CODE CWfdWlanSolution::GetSoftAPTemporaryProfile(CString strSSID ,CString strSecondaryKey, CString & strPrifile)
{
    strPrifile = L"<?xml version=\"1.0\" encoding=\"US-ASCII\"?>\
					<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\
						<name>" + strSSID + L"</name>\
						<SSIDConfig>\
							<SSID>\
								<name>" + strSSID + L"</name>\
							</SSID>\
						</SSIDConfig>\
						<connectionType>ESS</connectionType>\
						<connectionMode>auto</connectionMode>\
						<autoSwitch>false</autoSwitch>\
						<MSM>\
							<security>\
								<authEncryption>\
									<authentication>WPA2PSK</authentication>\
									<encryption>AES</encryption>\
									<useOneX>false</useOneX>\
								</authEncryption>\
								<sharedKey>\
									<keyType>passPhrase</keyType>\
									<protected>false</protected>\
									<keyMaterial>" + strSecondaryKey + L"</keyMaterial>\
								</sharedKey>\
							</security>\
						</MSM>\
					</WLANProfile>";
    return WFD_ERROR_SUCCESS;
}


WFD_ERROR_CODE CWfdWlanSolution::UpdateIPAddress()
{  
    size_t nConvertSize = 0;
    wchar_t wbuf[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
    ULONG ulMemSize = 0;

    // GetAdapterInfo
    PIP_ADAPTER_INFO pIpAdapterInfo = NULL; 
    PIP_ADAPTER_INFO pIpAdapterInfoCur = NULL;	
    GetAdaptersInfo(NULL, &ulMemSize);

    char *pMem = new char[ulMemSize];
    pIpAdapterInfo = (PIP_ADAPTER_INFO)pMem;
    GetAdaptersInfo(pIpAdapterInfo, &ulMemSize);
    pIpAdapterInfoCur = pIpAdapterInfo;

    in_addr gateway_addr;
    bool bIsInterfaceFound = false;
    for (; NULL != pIpAdapterInfoCur; pIpAdapterInfoCur = pIpAdapterInfoCur->Next)
    {
        memset(wbuf, 0, sizeof(wbuf));
        mbstowcs_s(&nConvertSize, 
            wbuf, 
            MAX_ADAPTER_DESCRIPTION_LENGTH + 4, 
            pIpAdapterInfoCur->Description, 
            MAX_ADAPTER_DESCRIPTION_LENGTH + 3);
        if(0 == wcscmp(wbuf, m_usedWlanInterface.strInterfaceDescription))
        {
            // find interface, get gateway ip, and update it to UI
            gateway_addr.S_un.S_addr = inet_addr(pIpAdapterInfoCur->GatewayList.IpAddress.String);
            m_WifiConnectedGatewayIP = gateway_addr; 
            gateway_addr.S_un.S_addr = inet_addr(pIpAdapterInfoCur->IpAddressList.IpAddress.String);
            m_WifiConnectedLocalIP = gateway_addr;
            bIsInterfaceFound = true;
            break;
        }
    }
    delete [] pMem;
    if(false == bIsInterfaceFound)
    {
        return WFD_ERROR_WLAN_GET_IP_FAILED;
    }
    return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdWlanSolution::DisConnectToAP()
{
	int rst = -1;
    //if (wlan_interface_state_disconnected != m_usedWlanInterface.isState)
    {
        rst = WlanDisconnect(m_hWlanClient, &(m_usedWlanInterface.InterfaceGuid), NULL);
    }
	return (WFD_ERROR_CODE)rst;
}

WFD_ERROR_CODE CWfdWlanSolution::ConnectToAP(IN unsigned int nBssEntryIndex, IN CString strSecondaryKey, OUT in_addr & ip_local_address, OUT in_addr & ip_gateway_address)
{
    if (WFD_ERROR_WLAN_ACQUIRE_HANDLE_FAILED == CheckWlanHandle())
    {
        return WFD_ERROR_WLAN_CONNECT_FAILED;
    }

   // if (WFD_ERROR_WLAN_REGISTER_MESSAGE_HANDLER_FAILED == RegisterNotificationFunc())
    {
   //     return WFD_ERROR_WLAN_CONNECT_FAILED;
    }	

	if (m_vectorSelectedBSSEntry.size() <= nBssEntryIndex)
	{
		return WFD_ERROR_INVALID_PARAMS;

	}

 //   if (wlan_interface_state_disconnected != m_usedWlanInterface.isState)
    {
	DWORD dwRet = 0;
    dwRet =  WlanDisconnect(m_hWlanClient, &(m_usedWlanInterface.InterfaceGuid), NULL);
    }

    WLAN_CONNECTION_PARAMETERS	WlanConnParam;

    // General WlanConnParam
	WlanConnParam.wlanConnectionMode = wlan_connection_mode_temporary_profile;

    UCHAR ucBuf[256] = {0};
	memcpy(ucBuf, m_vectorSelectedBSSEntry[nBssEntryIndex].dot11Ssid.ucSSID, m_vectorSelectedBSSEntry[nBssEntryIndex].dot11Ssid.uSSIDLength);
	CString strSSID(ucBuf);

	
	CString strPrifile;
	GetSoftAPTemporaryProfile(strSSID, strSecondaryKey, strPrifile);


	// No network profile specified for now
	WlanConnParam.strProfile = strPrifile;

	// SSID(Name) of the target AP
	WlanConnParam.pDot11Ssid = NULL;

	// dot11_BSS_type_infrastructure or dot11_BSS_type_independent
	WlanConnParam.dot11BssType = m_vectorSelectedBSSEntry[nBssEntryIndex].dot11BssType;	

	WlanConnParam.pDesiredBssidList = NULL;
	WlanConnParam.dwFlags = 0;

    DWORD dwError = 0;
    dwError = WlanConnect( m_hWlanClient, &(m_usedWlanInterface.InterfaceGuid), &WlanConnParam, NULL);
	if( ERROR_SUCCESS != dwError )
	{
		AfxMessageBox(_T("The device is not avaiable, try to refresh the list"),MB_OK);
        return WFD_ERROR_WLAN_CONNECT_FAILED;
	}

    // wait for connect success
    SetConnectConditionCriticalSectionValue(0);
    unsigned int nCondition = 0;
    unsigned int nWhileCount = 0;
    while(true)
    {
        nWhileCount ++;
        // Timeout is 8s(40 * 200 milliseconds)
        if(nWhileCount > 80)
        {
            nCondition = 0;
            break;
        }        
        
        nCondition = GetConnectConditionCriticalSectionValue();
        if(0 == nCondition)            
        {
            Sleep(200);
        }
        else
        {
            break;
        }
    }
    if (0 == nCondition)
    {
        AfxMessageBox(_T("ERROR_WLAN_CONNECT_TIMEOUT"),MB_OK);
        return WFD_ERROR_WLAN_CONNECT_TIMEOUT;
    }

    if (UpdateIPAddress())
    {
        return WFD_ERROR_WLAN_GET_IP_FAILED;
    }
    if (strcmp("0.0.0.0",  inet_ntoa(m_WifiConnectedLocalIP)) == 0)
    {
		AfxMessageBox(_T("The device is not avaiable, try to refresh the list"),MB_OK);
        return WFD_ERROR_WLAN_GET_IP_FAILED;
    }
	int times = 0;
	while (m_WifiConnectedLocalIP.S_un.S_un_b.s_b1 == 169)
	{
		times++;
		Sleep(2000);
		UpdateIPAddress();	
		if (10 == times)
		{
			break;
		}
	}
    ip_local_address = m_WifiConnectedLocalIP;
    ip_gateway_address = m_WifiConnectedGatewayIP;
    return WFD_ERROR_SUCCESS;
}





// - NIC issue -

DWORD CWfdWlanSolution::
AthEnumQcaWiFiDevices( 
    __out		LPTSTR      DevPaths,
	__in __out	PULONG		pPathLength,
    __out		PULONG      pNumDevFound
    )
{
    DWORD   dwRet = ERROR_SUCCESS;
    LPTSTR  pDevNames;
    LPTSTR  *pStringArray;
    ULONG   NumOfStrings = 0;
    ULONG   i;
	ULONG	SubStrLength;
	ULONG	OutputStrIndex;
	ULONG	QcaSubStrFound;

    if( NULL == DevPaths || NULL == pPathLength) 
    {
        return ERROR_INVALID_PARAMETER;
    }

	pDevNames = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * QCA_MAX_IOCTL_BUFFER);
	if( NULL == pDevNames )
	{
		return ERROR_NOT_ENOUGH_MEMORY;
	}

    dwRet = QueryDosDevice( NULL, pDevNames, QCA_MAX_IOCTL_BUFFER );
    if( 0 == dwRet ) 
    {
        dwRet = GetLastError();
        return dwRet;
    }

    if( dwRet > QCA_MAX_IOCTL_BUFFER )
	{
		return ERROR_BUFFER_OVERFLOW;
	}

    dwRet = SplitMultiString( pDevNames, &pStringArray, &NumOfStrings );

    if( dwRet != ERROR_SUCCESS ) 
    {
        return dwRet;
    }

    for( i = 0, OutputStrIndex = 0, QcaSubStrFound = 0; i < NumOfStrings; i++ ) 
	{
        if( AthIsQcaWiFiDevice( pStringArray[i] ) ) 
        {
			SubStrLength = wcslen( pStringArray[i] );
			if( OutputStrIndex + SubStrLength < *pPathLength )
			{
				_tcscpy_s(&DevPaths[OutputStrIndex], SubStrLength + 1, pStringArray[i]);
			}

			OutputStrIndex += (SubStrLength + 1);
			QcaSubStrFound += 1;
        }
    }

    if( *pPathLength <= OutputStrIndex )
    {
        *pPathLength = OutputStrIndex + 1;
        dwRet = ERROR_INSUFFICIENT_BUFFER;
    }
	else
	{
		DevPaths[OutputStrIndex] = _T('\0');
	}

	if( pNumDevFound )
	{
		*pNumDevFound = QcaSubStrFound;
	}

    LocalFree( pStringArray );

    return dwRet;
}


DWORD CWfdWlanSolution::
AthOpenWiFiDevice(
    __in        LPCTSTR     DevPath,
    __in __out  PHANDLE     pDeviceHandle
    )
/*
    Parameters:
        DevPath     : String of the target device's instance name
        pDevHandle  : Buffer to store the device handle if opened successfully
 
    Return value   
*/
{
    DWORD   dwRet = ERROR_SUCCESS;
    HANDLE  hDevice;
    ULONG   DevPathLength;
    ULONG   PrefixLength = _tcslen(WIN_DEV_INST_PREFIX);
    LPTSTR  TargetDevPath = NULL;

    if(  NULL == DevPath || NULL == pDeviceHandle )
    {
        return ERROR_INVALID_PARAMETER;
    }

    DevPathLength = _tcslen( DevPath );

    TargetDevPath = (LPTSTR)LocalAlloc( LPTR, sizeof(TCHAR) * ( PrefixLength + DevPathLength + 1 ) );

    if( NULL == TargetDevPath )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
	_tcscpy_s(TargetDevPath, _tcslen(WIN_DEV_INST_PREFIX) + 1, WIN_DEV_INST_PREFIX);
	_tcscat_s(TargetDevPath, _tcslen(WIN_DEV_INST_PREFIX) + DevPathLength + 1, DevPath);

    hDevice = CreateFile( 
                TargetDevPath,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

	LocalFree( TargetDevPath );

    if( INVALID_HANDLE_VALUE == hDevice ) 
    {
		dwRet = GetLastError();
		return dwRet;
	}
    
    if(  pDeviceHandle )
    {
        *pDeviceHandle = hDevice;
    }

    return ERROR_SUCCESS;
}


DWORD CWfdWlanSolution::
AthCloseWiFiDevice(
    __in    HANDLE      hWiFiDevice
    )
{
    if(  hWiFiDevice )
    {
        CloseHandle( hWiFiDevice );
    }

    return ERROR_SUCCESS;
}



DWORD CWfdWlanSolution::
AthWiFiDeviceIoControl(
    __in    HANDLE      hWiFiDevice,
    __in    DWORD       dwIoControlCode,
    __in    LPVOID      lpInBuffer,
    __in    ULONG       lInBufferSize,
    __out   LPVOID      lpOutBuffer,
    __out   ULONG       lOutBufferSize,
    __in    ULONG       lFlags,
    __out   PULONG      pBytesReturned
    )
{
    ULONG	BytesReturned;
    BOOL	bRet;
    DWORD   dwRet;

    bRet = DeviceIoControl( 
				hWiFiDevice, 
				dwIoControlCode, 
				lpInBuffer, 
				lInBufferSize, 
				lpOutBuffer,
				lOutBufferSize,
				&BytesReturned,
				NULL 
				);
    if( !bRet )
    {
		dwRet = GetLastError();
        return dwRet;
    }

	if( pBytesReturned )
	{
		*pBytesReturned = BytesReturned;
	}

    return ERROR_SUCCESS;
}



BOOL CWfdWlanSolution::
AthIsQcaWiFiDevice(
    __in    LPTSTR     DevPath
    )
{
    BOOL    bRet = FALSE;
    TCHAR   *ptCharDest;

    if(  NULL == DevPath || _T('\0') == DevPath[0] )
    {
        return FALSE;
    }

    MakeStrUpperCase( DevPath );

    ptCharDest = _tcsstr( DevPath, ATH_JANUS_ID );
    if(  NULL != ptCharDest )
    {
        return TRUE;
    }

    ptCharDest = _tcsstr( DevPath, ATH_SDIO_ID );
    if(  NULL != ptCharDest )
    {
        return TRUE;
    }

    ptCharDest = _tcsstr( DevPath, ATH_JANUS_USB_ID );
    if(  NULL != ptCharDest )
    {
        return TRUE;
    }

    return FALSE;
}


DWORD CWfdWlanSolution::
SplitMultiString(
    __in    LPCTSTR     MultiString,
    __out   LPTSTR      **StringArray,
    __out   PULONG      NumOfStringReturned
    )
{
    ULONG	i;
	DWORD	dwRet = ERROR_SUCCESS;
	ULONG	NumOfStrings, StartOfString;
	LPTSTR	*pStringArray;

	if( NULL == MultiString )
	{
		return ERROR_INVALID_PARAMETER;
	}

	if( NULL == StringArray || NULL == NumOfStringReturned )
	{
		return ERROR_INVALID_PARAMETER;
	}

	/*
		Count the number of sub strings contained in MultiString
	*/
	NumOfStrings = 0;
	i = 0;
	while( !( MultiString[i] == _T('\0') && MultiString[i+1] == _T('\0') ) )
	{
		// Skip the prefixing _T('\0')
		while( _T('\0') == MultiString[i] )
		{
			++i;
		}

		StartOfString = i;

		while( MultiString[i] != _T('\0') )
		{
			++i;
		}

		++NumOfStrings;
	}

	if( 0 == NumOfStrings )
	{
		*StringArray = NULL;
		*NumOfStringReturned = 0;
		return ERROR_SUCCESS;
	}

	/*
		Construct the sub string array
	*/
	pStringArray = (LPTSTR *)LocalAlloc(LPTR, sizeof(LPTSTR) * NumOfStrings );
	if( NULL == pStringArray )
	{
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	NumOfStrings = 0;
	i = 0;
	while( !( MultiString[i] == _T('\0') && MultiString[i+1] == _T('\0') ) )
	{
		// Skip the prefixing _T('\0')
		while( _T('\0') == MultiString[i] )
		{
			++i;
		}

		StartOfString = i;

		while( MultiString[i] != _T('\0') )
		{
			++i;
		}

		pStringArray[NumOfStrings] = (LPTSTR)&MultiString[StartOfString];
		++NumOfStrings;
	}

	*StringArray = pStringArray;
	*NumOfStringReturned = NumOfStrings;
	
	return ERROR_SUCCESS;
}



VOID CWfdWlanSolution::
MakeStrUpperCase( 
    __in __out  LPTSTR  DevPath 
    )
{
    ULONG	i;

    if( NULL == DevPath )
    {
        return;
    }

	i = 0;
    while( DevPath[i] != _T('\0') )
    {
        if( _istlower( DevPath[i] ) )
        {
            DevPath[i] = _T('A') + DevPath[i] - _T('a');
        }

		++i;
    }

    return;
}

WFD_ERROR_CODE CWfdWlanSolution::SetAtherosNICInfomationElement(bool bIsSetSepcificInfo)
{
	DWORD	dwRet, dwError;
	ULONG	BufferLength;
	ULONG	NumQcaDevFound;
	ULONG	NumDevFound;
	ULONG	i;
	PWCHAR  pDevNames;
	HANDLE  hDevice;
	LPTSTR  *pQcaDevStrings;

    // Alloc buffer
	PUCHAR	pBuffer = (PUCHAR)LocalAlloc( LPTR, sizeof(UCHAR) * QCA_MAX_IOCTL_BUFFER );	
	
	if( NULL == pBuffer )
	{
		return WFD_ERROR_WLAN_SET_ATHEROS_NIC_IE_FAILED;
	}

    // enum all qca device
	BufferLength = QCA_MAX_IOCTL_BUFFER;
    dwRet = AthEnumQcaWiFiDevices(
                (LPTSTR)pBuffer,
                &BufferLength,
                &NumDevFound
                );

    if( ERROR_SUCCESS != dwRet )
    {
		return WFD_ERROR_WLAN_SET_ATHEROS_NIC_IE_FAILED;
    }

    
	if( NumDevFound )
	{
		pDevNames = (LPTSTR)pBuffer;
		dwRet = SplitMultiString(
					pDevNames,
					&pQcaDevStrings,
					&NumQcaDevFound
					);
		/*
		CString strTemp;
		strTemp.Format(L"Get %d Device", NumDevFound);
		AfxMessageBox(strTemp);
        */
		

	}
	else
	{		
		// AfxMessageBox(L"Not Found QCA Device.");		
		return WFD_ERROR_WLAN_SET_ATHEROS_NIC_IE_FAILED;
	}

	/*ULONG NdisOid = OID_ATH_SMARTANT_INVITE;
	BYTE buffer[INFORMATION_ELEMENT_SIZE];
	memset(buffer, 0, INFORMATION_ELEMENT_SIZE);
    if(bIsSetSepcificInfo)
    {
        memcpy(buffer, QUALCOMM_WIFI_DISPLAY_ANSI_IDENTIFICATION, sizeof(QUALCOMM_WIFI_DISPLAY_ANSI_IDENTIFICATION));
    }	
	*/

	////////////////////////////////test
	ULONG NdisOid = OID_DOT11_CURRENT_ADDRESS;
	BYTE buffer[INFORMATION_ELEMENT_SIZE];
	memset(buffer, 0, INFORMATION_ELEMENT_SIZE);
	//
	ULONG bufferSize = INFORMATION_ELEMENT_SIZE;
	DWORD dwBytesReturned;

	// Set to all QCA Device
	for( i = 0; i < NumDevFound; i++ )
	{
		dwRet = AthOpenWiFiDevice(
					pQcaDevStrings[i],
					&hDevice
					);
		if(ERROR_SUCCESS == dwRet)
		{

			dwError = AthWiFiDeviceIoControl(
					hDevice,
					IOCTL_NDIS_QUERY_GLOBAL_STATS,
					&NdisOid,
					sizeof(NdisOid),
					buffer,
					bufferSize,
					0,
					&dwBytesReturned
					);
            if (ERROR_SUCCESS != dwError)
            {
                /*
                CString strError;
                strError.Format(L"AthWiFiDeviceIoControl() call Failed, Error code :%d", dwError);
                AfxMessageBox(strError);
                */

            }
			AthCloseWiFiDevice(hDevice);
		}
        else
        {
            /*
            CString strTemp;
		    strTemp.Format(L"Open Device %s failed. Error code : %d", pQcaDevStrings[i], dwRet);
		    AfxMessageBox(strTemp);
            */
            return WFD_ERROR_WLAN_SET_ATHEROS_NIC_IE_FAILED;
        }
	}


    return WFD_ERROR_SUCCESS;
}

bool CWfdWlanSolution::isPasswordNeeded(int nIndex)
{
#ifdef _DEBUG
    assert(nIndex >= 0);
    assert(nIndex < m_vectorSelectedBSSEntry.size());
#endif
    return m_vectorSelectedBSSEntry[nIndex].usCapabilityInformation & 0x1? true: false;
}

void CWfdWlanSolution::GuidToString(GUID* Guid, char* GUIDString)
{
	unsigned int count=0;
	GUIDString[count++]='{';
	for(int i=0;i<8;i++)
	{
		char tmp = (Guid->Data1 >> i*4) & 0xf;
		if(tmp<10)
			GUIDString[count+8-i-1]= tmp+ '0';
		else if(tmp<=0xf)
			GUIDString[count+8-i-1]= tmp - 0xa + 'A';
	}
	count+=8;
	GUIDString[count++]='-';

	for(int i=0;i<4;i++)
	{
		char tmp = (Guid->Data2 >> i*4) & 0xf;
		if(tmp<10)
			GUIDString[count+4-i-1]= tmp+ '0';
		else if(tmp<=0xf)
			GUIDString[count+4-i-1]= tmp - 0xa + 'A';
	}

	count+=4;
	GUIDString[count++]='-';

	for(int i=0;i<4;i++)
	{
		char tmp = (Guid->Data3 >> i*4) & 0xf;
		if(tmp<10)
			GUIDString[count+4-i-1]= tmp + '0';
		else if(tmp<=0xf)
			GUIDString[count+4-i-1]= tmp - 0xa + 'A';
	}

	count+=4;
	GUIDString[count++]='-';

	unsigned int data4Count=0;
	
	for(int i=0;i<2;i++)
	{
		char tmp =((Guid->Data4[data4Count]) >> 4) & 0xf;
		if(tmp<10)
			GUIDString[count++]= tmp + '0';
		else if(tmp<=0xf)
			GUIDString[count++]= tmp - 0xa + 'A';

		tmp = (Guid->Data4[data4Count]) & 0xf;
		if(tmp<10)
			GUIDString[count++]= tmp + '0';
		else if(tmp<=0xf)
			GUIDString[count++]= tmp - 0xa + 'A';

		data4Count++;
	}

	GUIDString[count++]='-';

	for(int i=0;i<6;i++)
	{
		char tmp =((Guid->Data4[data4Count]) >> 4) & 0xf;
		if(tmp<10)
			GUIDString[count++]= tmp + '0';
		else if(tmp<=0xf)
			GUIDString[count++]= tmp - 0xa + 'A'; 
		
		tmp = (Guid->Data4[data4Count]) & 0xf;
		if(tmp<10)
			GUIDString[count++]= tmp + '0';
		else if(tmp<=0xf)
			GUIDString[count++]= tmp - 0xa + 'A';

		data4Count++;
	}
	GUIDString[count++] = '}';
	GUIDString[count] = '\0';
}

bool CWfdWlanSolution::IsSameGUID(char* strGuid, GUID* Guid)
{
	char GUIDString[GUID_STR_LEN]={0};
	GuidToString(Guid, GUIDString);

	if(memcmp(strGuid, GUIDString, sizeof(GUIDString)) != 0 )
		return false;
	else
		return true;
}

WFD_ERROR_CODE CWfdWlanSolution::getVirtualAdapterGUID(char* strVirtualAdatperGUID)
{
	WFD_ERROR_CODE ret;
	GUID EmptyGUID;
	memset(&EmptyGUID,0,sizeof(GUID));
	if( IsSameGUID(m_VirtualAdapterGUID, &EmptyGUID) )
	{
		if((ret=EnumVirtualAdapterInterfaceGUID()) != WFD_ERROR_SUCCESS)
			return ret;
	}
	memcpy(strVirtualAdatperGUID, m_VirtualAdapterGUID, GUID_STR_LEN);
	return WFD_ERROR_SUCCESS;
}

WFD_ERROR_CODE CWfdWlanSolution::getWlanAdapterGUID(char* strWlanAdatperGUID)
{
	WFD_ERROR_CODE ret;
	GUID EmptyGUID;
	memset(&EmptyGUID,0,sizeof(GUID));
	if( IsSameGUID(m_WlanInterfaceGUID, &EmptyGUID) )
	{
		if((ret=EnumWlanAdapterInterfaceGUID()) != WFD_ERROR_SUCCESS)
			return ret;
	}
	memcpy(strWlanAdatperGUID, m_WlanInterfaceGUID, GUID_STR_LEN);
	return WFD_ERROR_SUCCESS;
}