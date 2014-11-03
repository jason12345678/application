// MsgHook.h : interface of the CMsgHook class
//
/////////////////////////////////////////////////////////////////////////////

#include <afxtempl.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define UM_HIDEEDITOR				WM_USER + 10
/////////////////////////////////////////////////////////////////////////////
// CMsgHook class

class CMsgHook
{
public:
	CMsgHook();
	virtual ~CMsgHook();

	BOOL Detach(void);
	BOOL Attach(HWND hWnd, HWND hParent, HWND hNotify = NULL);
	inline BOOL IsAttached(){return m_hWnd != NULL;}

protected:
	HWND m_hWnd;
	HWND m_hParent;
	HWND m_hNotify;
	BOOL m_bDropDown;
	static HHOOK m_hHook;
	static int m_nInstances;
	static CMap<HWND,HWND&,CMsgHook*,CMsgHook*&>m_MapHookedWindows;  // map of windows to hooks
	static LRESULT CALLBACK CallWndRetProc(int nCode,WPARAM wParam,LPARAM lParam);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
