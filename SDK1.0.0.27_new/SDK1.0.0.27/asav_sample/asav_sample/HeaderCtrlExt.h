// HeaderCtrlExt.h : interface of the CHeaderCtrlExt class
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern UINT WM_HDN_ENDDRAG;
// added for Visual 6.xx (lSalters)...
typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

/////////////////////////////////////////////////////////////////////////////
// CHeaderCtrlExt

class CHeaderCtrlExt : public CHeaderCtrl
{
	DECLARE_DYNAMIC(CHeaderCtrlExt)

public:
	class CItemData
	{
	public:
		CItemData(int nWidth, BOOL bRemovable, BOOL bVisible)
		{
			m_nWidth = nWidth;
			m_bVisible = bVisible;
			m_bRemovable = bRemovable;
		}
		int m_nWidth;
		BOOL m_bVisible;
		BOOL m_bRemovable;
	};

public:
	CHeaderCtrlExt();
	virtual ~CHeaderCtrlExt();

	int GetItemWidth(int nIndex);
	int IndexToOrder(int nIndex);
	int FindVisibleItem(int nIndex);
	BOOL SetItemWidth(int nIndex, int nWidth);
	BOOL GetWidthArray(int* piArray, int nCount);
	BOOL SetWidthArray(int nCount, int* piArray);
	DWORD_PTR GetItemData(int nIndex);
	BOOL SetItemData(int nIndex, DWORD_PTR dwData);
	BOOL GetVisibleArray(int* piArray, int nCount);
	BOOL SetVisibleArray(int nCount, int* piArray);
	BOOL GetVisible(int nIndex);
	void SetVisible(int nIndex, BOOL bVisible);
	BOOL GetRemovable(int nIndex);
	void SetRemovable(int nIndex, BOOL bRemovable);
	void ResetItemWidth(int nIndex);
	BOOL SetOrderArray(int nCount, LPINT piArray);
	int GetVisibleItemCount();
	int GetRemovableItemCount();

protected:
	BOOL m_nItemWidth;
	CPoint m_ptDragEnd;
	CPoint m_ptDragMove;
	CPoint m_ptDragStart;
	int m_nDraggingItem;
	BOOL m_bDragFullWindow;

protected:
	void DrawDragDivider();
	void BeginDragDivider();
	int	ItemFromPoint(CPoint point);
	void EndDragDivider(BOOL bSubmit);

protected:
	//{{AFX_MSG(CHeaderCtrlExt)
	afx_msg void OnDestroy();
	afx_msg void OnCaptureChanged(CWnd* pWnd);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	afx_msg LRESULT OnHdnEndDrag(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
