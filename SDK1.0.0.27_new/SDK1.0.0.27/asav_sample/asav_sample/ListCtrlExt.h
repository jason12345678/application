// ListCtrlExt.h : interface of the CListCtrlExt class
//
/////////////////////////////////////////////////////////////////////////////

#include <Afxmt.h>
#include "MsgHook.h"
#include "HeaderCtrlExt.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListCtrlExt.h : header file
//

typedef BOOL (*PFNEDITORCALLBACK)(CWnd** pWnd, int nRow, int nColumn, CString& strSubItemText, DWORD_PTR dwItemData, void* pThis, BOOL bUpdate);
/////////////////////////////////////////////////////////////////////////////
// CListCtrlExt

class CListCtrlExt : public CListCtrl
{
	DECLARE_DYNAMIC(CListCtrlExt)

public:
	HACCEL m_hAccel;
	BOOL SaveState();
	BOOL RestoreState();
	BOOL IsColumnHidden(int nColumn);

	inline int GetColumnSort(){return m_nSortColumn;}
	CHeaderCtrlExt* GetHeaderCtrl(){return m_pHeaderCtrl;}
	void SetGridBehaviour(BOOL bGrid = TRUE)
	{
		if(bGrid && (GetStyle() & LVS_TYPEMASK) == LVS_REPORT)m_bGrid = bGrid;
		else m_bGrid = FALSE;
	}

public:
	DWORD_PTR GetItemData(int nItem) const;
	BOOL SetItemData(int nItem, DWORD_PTR dwData);

	BOOL EnsureSubItemVisible(int nItem, int nSubItem, CRect* pRect = NULL);
	typedef enum Sort
	{
		None = 0,
		Ascending = 1,
		Descending = 2,
		Auto = 4,
		SortBits = 7
	}Sort;
	typedef enum Comparer
	{
		NotSet,
		Int,
		Double,
		String,
		StringNumber,
		StringNoCase,
		StringNumberNoCase,
		Date
	}Comparer;

// Operations
public:
	CListCtrlExt();
	virtual ~CListCtrlExt();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCtrlExt)
	public:
	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	int GetColumnCount();
	void SetColumnEditor(int nColumn, CWnd* pWnd);
	void SetColumnEditor(int nColumn, PFNEDITORCALLBACK pfnInitEditor, PFNEDITORCALLBACK m_pfnEndEditor = NULL,  CWnd* pWnd = NULL);
	void SetCellEditor(int nRow, int nColumn, CWnd* pWnd);
	void SetCellEditor(int nRow, int nColumn, PFNEDITORCALLBACK pfnInitEditor, PFNEDITORCALLBACK m_pfnEndEditor = NULL,  CWnd* pWnd = NULL);
	void SetRowEditor(int nRow, CWnd* pWnd);
	void SetRowEditor(int nRow, PFNEDITORCALLBACK pfnInitEditor, PFNEDITORCALLBACK m_pfnEndEditor = NULL,  CWnd* pWnd = NULL);
	void SetDefaultEditor(CWnd* pWnd);
	void SetDefaultEditor(PFNEDITORCALLBACK pfnInitEditor, PFNEDITORCALLBACK m_pfnEndEditor = NULL,  CWnd* pWnd = NULL);
	void SetColumnReadOnly(int nColumn, BOOL bReadOnly = TRUE);
	void SetCellReadOnly(int nRow, int nColumn, BOOL bReadOnly = TRUE);
	void SetRowReadOnly(int nRow, BOOL bReadOnly = TRUE);
	BOOL IsColumnReadOnly(int nColumn);
	BOOL IsRowReadOnly(int nRow);
	BOOL IsCellReadOnly(int nRow, int nColumn);

	BOOL DisplayEditor(int nItem, int nSubItem);
	void HideEditor(BOOL bUpdate = TRUE);

	void SetCellColors(int nRow, int nColumn, COLORREF clrBack, COLORREF clrText);
	void SetRowColors(int nItem, COLORREF clrBk, COLORREF clrText);
	void SetColumnColors(int nColumn, COLORREF clrBack, COLORREF clrText);

	BOOL Reset();
	void SelectItem(int nItem, BOOL bSelect);
	BOOL AddItem(int nItemIndex, int nSubItemIndex, LPCTSTR lpszItemText, int nImageIndex = -1);
	void DeleteSelectedItems();
	void HandleDeleteKey(BOOL bHandle = TRUE);
	BOOL DeleteAllColumns();

	BOOL SortOnColumn(int nColumn, BOOL bChangeOrder = FALSE);
	BOOL SortItems(PFNLVCOMPARE pfnCompare, DWORD_PTR dwData);
	void SetColumnSorting(int nColumn, Sort eSort, PFNLVCOMPARE fnCallBack);
	void SetColumnSorting(int nColumn, Sort eSort, Comparer eSortType = String);

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CMsgHook m_MsgHook;
	BOOL m_bHandleDelete;
	DWORD_PTR m_dwSortData;
	PFNLVCOMPARE m_fnCompare;
	CCriticalSection m_oLock;
	CHeaderCtrlExt* m_pHeaderCtrl;
	COLORREF m_clrDefBack, m_clrDefText;
	int m_nRow, m_nColumn, m_nEditingRow, m_nEditingColumn, m_nSortColumn;

protected:
	static int CompareInt(LPCTSTR pLeftText, LPCTSTR pRightText);
	static int CompareDate(LPCTSTR pLeftText, LPCTSTR pRightText);
	static int CompareDouble(LPCTSTR pLeftText, LPCTSTR pRightText);
	static int CompareString(LPCTSTR pLeftText, LPCTSTR pRightText);
	static int CompareStringNoCase(LPCTSTR pLeftText, LPCTSTR pRightText);
	static int CompareNumberString(LPCTSTR pLeftText, LPCTSTR pRightText);
	static int CompareNumberStringNoCase(LPCTSTR pLeftText, LPCTSTR pRightText);
	static int Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

protected:
	typedef struct EditorInfo
	{
		CWnd* m_pWnd;
		BOOL m_bReadOnly;
		PFNEDITORCALLBACK m_pfnEndEditor;
		PFNEDITORCALLBACK m_pfnInitEditor;
		EditorInfo();
		EditorInfo(PFNEDITORCALLBACK pfnInitEditor, PFNEDITORCALLBACK pfnEndEditor, CWnd* pWnd = NULL);
		inline BOOL IsSet(){return (m_pfnInitEditor || m_pWnd);}
	}EditorInfo;

	typedef struct CellInfo
	{	
		int m_nColumn;
		COLORREF m_clrBack;
		COLORREF m_clrText;	
		EditorInfo m_eiEditor;
		DWORD_PTR	m_dwUserData;
		CellInfo(int nColumn);
		CellInfo(int nColumn, COLORREF clrBack, COLORREF clrText, DWORD_PTR dwUserData = NULL);
		CellInfo(int nColumn, EditorInfo eiEditor, COLORREF clrBack, COLORREF clrText, DWORD_PTR dwUserData = NULL);
		CellInfo(int nColumn, EditorInfo eiEditor, DWORD_PTR dwUserData = NULL);
	}CellInfo;

	typedef struct ColumnInfo
	{
		Sort m_eSort;
		int m_nColumn;
		COLORREF m_clrBack;
		COLORREF m_clrText;	
		Comparer m_eCompare;
		EditorInfo m_eiEditor;
		PFNLVCOMPARE m_fnCompare;
		ColumnInfo(int nColumn);
		ColumnInfo(int nColumn, PFNEDITORCALLBACK pfnInitEditor, PFNEDITORCALLBACK pfnEndEditor, CWnd *pWnd = NULL);
	}ColumnInfo;

	typedef struct ItemData
	{
		EditorInfo m_eiEditor;
		COLORREF m_clrBack;
		COLORREF m_clrText;
		DWORD_PTR	m_dwUserData;
		ItemData(DWORD_PTR dwUserData = NULL);
		CArray<CellInfo*,CellInfo*> m_aCellInfo;
		ItemData(COLORREF clrBack, COLORREF clrText, DWORD_PTR dwUserData = NULL);
		inline BOOL IsSet()
		{
			return (m_dwUserData || m_clrBack != -1 || m_clrText != -1 || m_aCellInfo.GetSize() == 0);
		}		
		virtual ~ItemData();
	}ItemData;

	CPtrArray m_aItemData;
	CPtrArray m_aColumnInfo;
	EditorInfo* m_pEditor;
	EditorInfo m_eiDefEditor;

protected:
	BOOL DeleteAllItemsData();
	BOOL DeleteAllColumnInfo();
	BOOL DeleteItemData(int nItem);
	BOOL DeleteColumnInfo(int nIndex);
	void UpdateFocusCell(int nCol);
	void MoveFocusCell(BOOL bRight);
	DWORD_PTR GetItemDataInternal(int nItem) const;
	BOOL VerifyOrderArray(int* piArray, int nCount);
	BOOL GetCellRect(int nRow, int nCol, CRect& rect);
	BOOL EnsureColumnVisible(int nCol, bool bPartialOK);
	int GetItemIndexFromData(DWORD_PTR dwData);
	CListCtrlExt::ColumnInfo* GetColumnInfo(int nColumn);
	CellInfo* GetCellInfo(int nItem, int nSubItem);
	DWORD_PTR GetCellData(int nRow, int nColumn);
	BOOL SetCellData(int nItem, int nSubItem, DWORD_PTR dwData);

	// Generated message map functions
protected:
	//{{AFX_MSG(CListCtrlExt)
	afx_msg BOOL OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHdnEnddrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNmRclickHeader(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHdnDividerdblclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	afx_msg LRESULT OnFindItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInsertItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDeleteItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInsertColumn(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDeleteColumn(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDeleteAllItems(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bGrid;
	BOOL m_bColumnSort;
	int m_FocusCell;

	// Keyboard search
	int		m_nLastSearchRow;		// Last row matched in keyboard search
	int		m_nLastSearchCell;		// Last column used in keyboard search
	CTime	m_tLastSearchTime;		// Time of last search attempt for keyboard search
	CString m_sLastSearchString;	// Last search criteria for keyboard search
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
