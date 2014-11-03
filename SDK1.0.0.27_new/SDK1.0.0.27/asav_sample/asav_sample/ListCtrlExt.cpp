// ListCtrlExt.cpp : implementation of the CListCtrlExt class
//

#include "stdafx.h"
#include "ListCtrlExt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef HDF_SORTDOWN
#define HDF_SORTDOWN				0x0200
#endif
#ifndef HDF_SORTUP
#define HDF_SORTUP					0x0400
#endif
/////////////////////////////////////////////////////////////////////////////
// CListCtrlExt

CListCtrlExt::EditorInfo::EditorInfo()
	:m_pfnInitEditor(NULL)
	,m_pfnEndEditor(NULL)
	,m_pWnd(NULL)
	,m_bReadOnly(FALSE)
{
}

CListCtrlExt::EditorInfo::EditorInfo(PFNEDITORCALLBACK pfnInitEditor, PFNEDITORCALLBACK pfnEndEditor, CWnd *pWnd)
	:m_pfnInitEditor(pfnInitEditor)
	,m_pfnEndEditor(pfnEndEditor)
	,m_pWnd(pWnd)
	,m_bReadOnly(FALSE)
{
}

CListCtrlExt::CellInfo::CellInfo(int nColumn)
	:m_clrBack(-1)
	,m_clrText(-1)
	,m_dwUserData(NULL)
	,m_nColumn(nColumn)
{
}

CListCtrlExt::CellInfo::CellInfo(int nColumn, COLORREF clrBack, COLORREF clrText, DWORD_PTR dwUserData)
	:m_clrBack(clrBack)
	,m_clrText(clrText)
	,m_dwUserData(dwUserData)
	,m_nColumn(nColumn)
{
}

CListCtrlExt::CellInfo::CellInfo(int nColumn, EditorInfo eiEditor, COLORREF clrBack, COLORREF clrText, DWORD_PTR dwUserData)
	:m_clrBack(clrBack)
	,m_clrText(clrText)
	,m_dwUserData(dwUserData)
	,m_eiEditor(eiEditor)
	,m_nColumn(nColumn)
{
}

CListCtrlExt::CellInfo::CellInfo(int nColumn, EditorInfo eiEditor, DWORD_PTR dwUserData)
	:m_clrBack(-1)
	,m_clrText(-1)
	,m_dwUserData(dwUserData)
	,m_eiEditor(eiEditor)
	,m_nColumn(nColumn)
{
}

CListCtrlExt::ColumnInfo::ColumnInfo(int nColumn)
	:m_eiEditor()
	,m_clrBack(-1)
	,m_clrText(-1)
	,m_nColumn(nColumn)
	,m_eSort(None)
	,m_eCompare(NotSet)
	,m_fnCompare(NULL)
{
}

CListCtrlExt::ColumnInfo::ColumnInfo(int nColumn, PFNEDITORCALLBACK pfnInitEditor, PFNEDITORCALLBACK pfnEndEditor, CWnd *pWnd)
	:m_eiEditor(pfnInitEditor, pfnEndEditor, pWnd)
	,m_nColumn(nColumn)
	,m_clrBack(-1)
	,m_clrText(-1)
	,m_eSort(None)
	,m_eCompare(NotSet)
	,m_fnCompare(NULL)
{
}

CListCtrlExt::ItemData::ItemData(DWORD_PTR dwUserData)
	:m_clrBack(0xFFFFFFFF)
	,m_clrText(0xFFFFFFFF)
	,m_dwUserData(dwUserData)
{
}

CListCtrlExt::ItemData::ItemData(COLORREF clrBack, COLORREF clrText, DWORD_PTR dwUserData)
	:m_clrBack(clrBack)
	,m_clrText(clrText)
	,m_dwUserData(dwUserData)
{
}

CListCtrlExt::ItemData::~ItemData()
{
	while(m_aCellInfo.GetSize() > 0)
	{
		CellInfo *pInfo = (CellInfo*)m_aCellInfo.GetAt(0);
		m_aCellInfo.RemoveAt(0);
		delete pInfo;
	}
}

// CListCtrlExt
IMPLEMENT_DYNAMIC(CListCtrlExt, CListCtrl)

CListCtrlExt::CListCtrlExt()
	:m_pEditor(NULL)
	,m_hAccel(NULL)
	,m_nEditingRow(-1)
	,m_MsgHook()
	,m_nRow(-1)
	,m_nColumn(-1)
	,m_nSortColumn(-1)
	,m_fnCompare(NULL)
	,m_dwSortData(NULL)
	,m_bColumnSort(FALSE)
	,m_bGrid(FALSE)
	,m_FocusCell(-1)
	,m_nLastSearchCell(-1)
	,m_nLastSearchRow(-1)
	,m_nEditingColumn(-1)
	,m_bHandleDelete(FALSE)
	,m_pHeaderCtrl(NULL)
{
	m_pHeaderCtrl = new CHeaderCtrlExt;
}

CListCtrlExt::~CListCtrlExt()
{
	DeleteAllItemsData();
	DeleteAllColumnInfo();
	delete m_pHeaderCtrl;
}


BEGIN_MESSAGE_MAP(CListCtrlExt, CListCtrl)
	//{{AFX_MSG_MAP(CListCtrlExt)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY(HDN_ENDDRAG, 0, OnHdnEnddrag)
	ON_NOTIFY(NM_RCLICK, 0, OnNmRclickHeader)
	ON_NOTIFY(HDN_DIVIDERDBLCLICK, 0, OnHdnDividerdblclick)
	ON_NOTIFY_REFLECT_EX(NM_DBLCLK, CListCtrlExt::OnNMDblclk)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, CListCtrlExt::OnNMCustomdraw)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, CListCtrlExt::OnColumnclick)
	//}}AFX_MSG_MAP
	ON_MESSAGE(LVM_FINDITEM, OnFindItem)
	ON_MESSAGE(LVM_INSERTITEM, OnInsertItem)
	ON_MESSAGE(LVM_DELETEITEM, OnDeleteItem)
	ON_MESSAGE(LVM_INSERTCOLUMN, OnInsertColumn)
	ON_MESSAGE(LVM_DELETECOLUMN, OnDeleteColumn)
	ON_MESSAGE(LVM_DELETEALLITEMS, OnDeleteAllItems)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListCtrlExt diagnostics

#ifdef _DEBUG
void CListCtrlExt::AssertValid() const
{
	CListCtrl::AssertValid();
}

void CListCtrlExt::Dump(CDumpContext& dc) const
{
	CListCtrl::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CListCtrlExt message handlers

void CListCtrlExt::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();

	// TODO: Add your specialized code here and/or call the base class

	DWORD dwOldStyle = GetStyle();
	ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
	::SetWindowLong(m_hWnd, GWL_STYLE, dwOldStyle);

	if(m_pHeaderCtrl->m_hWnd == NULL)
	{
		HWND hWnd = (HWND)::SendMessage(m_hWnd,LVM_GETHEADER,0,0);
		m_pHeaderCtrl->SubclassWindow(hWnd);

		m_clrDefBack = GetTextBkColor() | 0xFF000000;
		m_clrDefText = GetTextColor();
	}
}

BOOL CListCtrlExt::PreTranslateMessage(MSG* pMsg)
{
	if((m_hAccel && GetParent() && GetFocus() == this && ::TranslateAccelerator(GetParent()->m_hWnd, m_hAccel, pMsg)))return TRUE;

	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		HideEditor();
		SetItemState(GetNextItem(-1, LVNI_FOCUSED), LVIS_SELECTED, LVIS_SELECTED);
	}

	return CListCtrl::PreTranslateMessage(pMsg);
}

LRESULT CListCtrlExt::OnInsertColumn(WPARAM wParam, LPARAM lParam)
{
	int nPos = (int)Default(); // call default control procedure

	LPLVCOLUMN pCol = (LPLVCOLUMN)lParam;
	CHeaderCtrlExt::CItemData* pData = new CHeaderCtrlExt::CItemData(pCol->cx, TRUE, TRUE);
	m_pHeaderCtrl->SetItemData((int)wParam, (DWORD_PTR)pData);

	return nPos;
}

LRESULT CListCtrlExt::OnDeleteColumn(WPARAM wParam, LPARAM lParam)
{
	CHeaderCtrlExt::CItemData* pData = (CHeaderCtrlExt::CItemData*)m_pHeaderCtrl->GetItemData((int)wParam);
	if(pData)delete pData;

	return Default();
}

LRESULT CListCtrlExt::OnInsertItem(WPARAM wParam, LPARAM lParam)
{
	int nPos = (int)Default();
	LPLVITEM pItem = (LPLVITEM)lParam;
	SetItemData(nPos, pItem->iItem);

	return nPos;
}

LRESULT CListCtrlExt::OnDeleteItem(WPARAM wParam, LPARAM lParam)
{
	DeleteItemData((int)wParam);

	return Default();
}

LRESULT CListCtrlExt::OnDeleteAllItems(WPARAM wParam, LPARAM lParam)
{
	DeleteAllItemsData();

	return Default();
}

LRESULT CListCtrlExt::OnFindItem(WPARAM wParam, LPARAM lParam)
{
	int nPos = (int)Default();
	LVFINDINFO* pLVFindItem = (LVFINDINFO*)lParam;
	if(pLVFindItem->flags & LVIF_PARAM)
	{
		int nStart = (int)wParam;
		int nCount = GetItemCount();
		for(int i = nStart + 1;i < nCount;++i)
		{
			if(pLVFindItem->lParam == (int)GetItemData(i))
			{
				nPos = i;
				break;
			}
		}
	}

	return nPos;
}

BOOL CListCtrlExt::EnsureSubItemVisible(int nItem, int nSubItem, CRect* pRect)
{
	BOOL bReturn = EnsureVisible(nItem, FALSE);
	CRect rect,rcList;
	GetClientRect(&rcList);
	GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rect);
	if(rect.right > rcList.Width())Scroll(CSize(rect.Width() > rcList.Width() ? rect.left : rect.right - rcList.Width(),0));
	if(rect.left < 0)Scroll(CSize(rect.left));
	if(pRect)
	{
		GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rect);
		rect.right = min(rect.right, rcList.Width() - 4);
		*pRect = rect;
	}

	return bReturn;
}

int CListCtrlExt::GetColumnCount()
{
	if(m_pHeaderCtrl)return m_pHeaderCtrl->GetItemCount();

	int i = 0;
	LVCOLUMN col;
	col.mask = LVCF_WIDTH;
	while(GetColumn(i++, &col));

	return i;
}

BOOL CListCtrlExt::AddItem(int nItemIndex, int nSubItemIndex, LPCTSTR lpszItemText, int nImageIndex)
{
	LV_ITEM lvItem;
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = nItemIndex;
	lvItem.iSubItem = nSubItemIndex;
	lvItem.pszText = (LPTSTR)lpszItemText;
	if(nImageIndex != -1)
	{
		lvItem.mask |= LVIF_IMAGE;
		lvItem.iImage |= LVIF_IMAGE;
	}

	if(nSubItemIndex == 0)return InsertItem(&lvItem);

	return SetItem(&lvItem);
}

void CListCtrlExt::SelectItem(int nItem, BOOL bSelect)
{
	int nIndex = -1;
	if(nItem >= 0)SetItemState(nItem, (bSelect ? LVIS_SELECTED: 0), LVIS_SELECTED);
	else
	{
		while((nIndex = GetNextItem(nIndex, bSelect ? LVNI_ALL : LVNI_SELECTED)) >= 0)
		{
			SetItemState(nIndex, (bSelect ? LVIS_SELECTED: 0), LVIS_SELECTED);
		}
	}
}

BOOL CListCtrlExt::Reset()
{
	return (DeleteAllItems() && DeleteAllColumns());
}

void CListCtrlExt::DeleteSelectedItems()
{
	int nItem = -1;
	while((nItem = GetNextItem(-1, LVNI_SELECTED)) >= 0)
		DeleteItem(nItem);
}

void CListCtrlExt::HandleDeleteKey(BOOL bHandle)
{
	m_bHandleDelete = bHandle;
}

DWORD_PTR CListCtrlExt::GetItemData(int nItem) const
{
	ItemData* pData = (ItemData*)GetItemDataInternal(nItem);
	if(! pData)return NULL;

	return pData->m_dwUserData;
}

DWORD_PTR CListCtrlExt::GetItemDataInternal(int nItem) const
{
	return CListCtrl::GetItemData(nItem);
}

BOOL CListCtrlExt::SetItemData(int nItem, DWORD_PTR dwData)
{
	ItemData* pData = (ItemData*)GetItemDataInternal(nItem);

	if(pData)pData->m_dwUserData = dwData;
	else
	{
		pData = new ItemData(dwData);
		m_aItemData.Add(pData);
	}

	return CListCtrl::SetItemData(nItem, (DWORD_PTR)pData);
}

int CListCtrlExt::GetItemIndexFromData(DWORD_PTR dwData)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = dwData;

	return CListCtrl::FindItem(&find);
}

BOOL CListCtrlExt::SetCellData(int nItem, int nSubItem, DWORD_PTR dwData)
{
	if(nItem < 0 || nItem >= GetItemCount() || nSubItem < 0 || nSubItem >= GetColumnCount())return FALSE;

	CellInfo* pCellInfo = GetCellInfo(nItem, nSubItem);
	if(! pCellInfo)
	{ 
		pCellInfo = new CellInfo(nSubItem);
		ItemData* pData = (ItemData*)GetItemDataInternal(nItem);
		if(! pData)
		{
			SetItemData(nItem, 0);
			pData = (ItemData*)GetItemDataInternal(nItem);
		}
		pData->m_aCellInfo.Add(pCellInfo);
	}
	pCellInfo->m_dwUserData = dwData;

	return TRUE;
}

DWORD_PTR CListCtrlExt::GetCellData(int nItem, int nSubItem)
{
	CellInfo* pCellInfo = GetCellInfo(nItem, nSubItem);

	if(pCellInfo)return pCellInfo->m_dwUserData;
	else return 0;
}

BOOL CListCtrlExt::DeleteAllColumns()
{
	while(DeleteColumn(0));

	return(GetColumnCount() == 0);
}

BOOL CListCtrlExt::DeleteAllItemsData()
{
	while(m_aItemData.GetSize() > 0)
	{
		ItemData* pData = (ItemData*)m_aItemData.GetAt(0);
		if(pData)delete pData;
		m_aItemData.RemoveAt(0);
	}

	return TRUE;
}

BOOL CListCtrlExt::DeleteItemData(int nItem)
{
	if(nItem < 0 || nItem > GetItemCount())return FALSE;

	ItemData* pData = (ItemData*)CListCtrl::GetItemData(nItem);
	INT_PTR nCount = m_aItemData.GetSize();

	for(INT_PTR i = 0; i < nCount && pData;++i)
	{
		if(m_aItemData.GetAt(i) == pData)
		{
			m_aItemData.RemoveAt(i);
			break;
		}
	}

	if(pData)delete pData;

	return TRUE;
}	

CListCtrlExt::ColumnInfo* CListCtrlExt::GetColumnInfo(int nColumn)
{
	INT_PTR nCount = m_aColumnInfo.GetSize();
	for(INT_PTR i = 0;i < nCount;++i)
	{
		ColumnInfo* pColInfo = (ColumnInfo*)m_aColumnInfo.GetAt(i);
		if(pColInfo->m_nColumn == nColumn)return pColInfo;
	}

	return NULL;
}

CListCtrlExt::CellInfo* CListCtrlExt::GetCellInfo(int nItem, int nSubItem)
{
	ItemData* pData = (ItemData*)GetItemDataInternal(nItem);
	if(pData == NULL)return NULL;
	INT_PTR nCount = pData->m_aCellInfo.GetSize();
	for(INT_PTR i = 0;i < nCount;++i)
	{
		CellInfo* pInfo = (CellInfo*)pData->m_aCellInfo.GetAt(i);
		if(pInfo && pInfo->m_nColumn == nSubItem)return pInfo;
	}

	return NULL;
}

BOOL CListCtrlExt::DeleteAllColumnInfo()
{
	while(m_aColumnInfo.GetSize() > 0)
	{
		ColumnInfo* pData = (ColumnInfo*)m_aColumnInfo.GetAt(0);
		if(pData)delete pData;
		m_aColumnInfo.RemoveAt(0);
	}

	return TRUE;
}

BOOL CListCtrlExt::DeleteColumnInfo(int nColumn)
{
	if(nColumn < 0 || nColumn > GetColumnCount())return FALSE;

	INT_PTR nCount = m_aColumnInfo.GetSize();
	ColumnInfo* pData = (ColumnInfo*)GetColumnInfo(nColumn);

	for(INT_PTR i = 0; i < nCount && pData;++i)
	{
		if(m_aColumnInfo.GetAt(i) == pData)
		{
			m_aColumnInfo.RemoveAt(i);
			break;
		}
	}

	if(pData)delete pData;

	return TRUE;
}

void CListCtrlExt::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVCUSTOMDRAW lplvcd = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);

	*pResult = 0;
	int iCol = lplvcd->iSubItem;
	int iRow = lplvcd->nmcd.dwItemSpec;
	int nRowItemData = (int)lplvcd->nmcd.lItemlParam;

	switch(lplvcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYSUBITEMDRAW;          // ask for subitem notifications.
			break;
		case CDDS_ITEMPREPAINT:
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
			if(lplvcd->nmcd.uItemState & CDIS_FOCUS)
			{
				// If drawing focus row, then remove focus state and request to draw it later
				//	- Row paint request can come twice, with and without focus flag
				//	- Only respond to the one with focus flag, else DrawFocusRect XOR will cause solid or blank focus-rectangle
				if(GetNextItem(-1, LVNI_FOCUSED) == iRow)
				{
					if(m_FocusCell >= 0)
					{
						// We want to draw a cell-focus-rectangle instead of row-focus-rectangle
						lplvcd->nmcd.uItemState &= ~CDIS_FOCUS;
						*pResult |= CDRF_NOTIFYPOSTPAINT;
					}
					else
					{
						// Avoid bug where bottom of focus rectangle is missing when using grid-lines
						//	- Draw the focus-rectangle for the entire row (explicit)
						lplvcd->nmcd.uItemState &= ~CDIS_FOCUS;
						*pResult |= CDRF_NOTIFYPOSTPAINT;
					}
				}
			}
			break;
		case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
			{
				COLORREF clrBack = 0xFFFFFFFF;
				COLORREF clrText = 0xFFFFFFFF;
				*pResult = CDRF_DODEFAULT;
				CellInfo *pCell = GetCellInfo(iRow, iCol);
				if(pCell)
				{
					clrBack = pCell->m_clrBack;
					clrText = pCell->m_clrText;
				}
				if(clrBack == 0xFFFFFFFF && clrText == 0xFFFFFFFF)
				{
					ItemData* pData = (ItemData*)GetItemDataInternal(iRow);
					if(pData)
					{
						clrBack = pData->m_clrBack;
						clrText = pData->m_clrText;
					}
				}
				if(clrBack == 0xFFFFFFFF && clrText == 0xFFFFFFFF)
				{
					ColumnInfo* pInfo = GetColumnInfo(iCol);
					if(pInfo)
					{						
						clrBack = pInfo->m_clrBack;
						clrText = pInfo->m_clrText;
					}
				}
				if(clrBack != 0xFFFFFFFF)
				{
					lplvcd->clrTextBk = clrBack;					
					*pResult  = CDRF_NEWFONT;
				}
				else
				{
					if(clrBack != m_clrDefBack)
					{
						lplvcd->clrTextBk = m_clrDefBack;				
						*pResult  = CDRF_NEWFONT;
					}
				}
				if(clrText != 0xFFFFFFFF)
				{					
					lplvcd->clrText = clrText;					
					*pResult  = CDRF_NEWFONT;
				}				
				else
				{
					if(clrText != m_clrDefText)
					{
						lplvcd->clrText = m_clrDefText;		
						*pResult  = CDRF_NEWFONT;
					}
				}
				// Remove the selection color for the focus cell, to make it easier to see focus
				if(lplvcd->nmcd.uItemState & CDIS_SELECTED && m_FocusCell == iCol && GetNextItem(-1, LVNI_FOCUSED) == iRow)
				{
					lplvcd->nmcd.uItemState &= ~CDIS_SELECTED;
				}
			}
			break;
		case CDDS_ITEMPOSTPAINT:
			if(m_bGrid && (GetExtendedStyle() & LVS_EX_FULLROWSELECT))
			{
				if(GetNextItem(-1, LVNI_FOCUSED) != iRow)break;
				// Perform the drawing of the focus rectangle
				if(m_FocusCell >= 0)
				{
					// Draw the focus-rectangle for a single-cell
					CRect rcHighlight;
					CDC* pDC = CDC::FromHandle(lplvcd->nmcd.hdc);
					VERIFY(GetCellRect(iRow, m_FocusCell, rcHighlight));
					// Adjust rectangle according to grid-lines
					int cxborder = ::GetSystemMetrics(SM_CXBORDER);
					// Columns after the first visible column, has to take account of left-grid-border
					if(m_pHeaderCtrl->OrderToIndex(m_FocusCell) != 0)rcHighlight.left += cxborder;
					rcHighlight.bottom -= cxborder;
					pDC->DrawFocusRect(rcHighlight);
				}
				else
				{
					// Avoid bug where bottom of focus rectangle is missing when using grid-lines
					//	- Draw the focus-rectangle for the entire row (explicit)
					CRect rcHighlight;
					CDC* pDC = CDC::FromHandle(lplvcd->nmcd.hdc);
					// Using LVIR_BOUNDS to get the entire row-rectangle
					VERIFY(GetItemRect(iRow, rcHighlight, LVIR_BOUNDS));
					int cxborder = ::GetSystemMetrics(SM_CXBORDER);
					rcHighlight.bottom -= cxborder;
					pDC->DrawFocusRect(rcHighlight);
				}
			}
			break;
		default:
			*pResult = CDRF_DODEFAULT;
			break;
	}
}

void CListCtrlExt::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* phdr = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	SortOnColumn(phdr->iSubItem, TRUE);
	*pResult = 0;
}

BOOL CListCtrlExt::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	if(m_bGrid && 
		(GetStyle() & LVS_TYPEMASK) == LVS_REPORT && 
		(GetExtendedStyle() & LVS_EX_FULLROWSELECT))
	{
		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
		if(pNMListView)
		{
			int nItem = pNMListView->iItem, nSubItem = pNMListView->iSubItem;
			*pResult = DisplayEditor(nItem, nSubItem);
		}
	}

	return *pResult;
}

void CListCtrlExt::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(! m_bGrid 
		|| (GetStyle() & LVS_TYPEMASK) != LVS_REPORT 
		|| ! (GetExtendedStyle() & LVS_EX_FULLROWSELECT))
	{
		CListCtrl::OnLButtonDown(nFlags, point);
		return;
	}

	// Find out what subitem was clicked
	LVHITTESTINFO hitinfo = {0};
	hitinfo.pt = point;
	hitinfo.flags = nFlags;
	SubItemHitTest(&hitinfo);

	// Update the focused cell before calling CListCtrl::OnLButtonDown()
	// as it might cause a row-repaint
	m_FocusCell = hitinfo.iSubItem;
	CListCtrl::OnLButtonDown(nFlags, point);

	// CListCtrl::OnLButtonDown() doesn't always cause a row-repaint
	// call our own method to ensure the row is repainted
	UpdateFocusCell(hitinfo.iSubItem);
}

void CListCtrlExt::OnRButtonDown(UINT nFlags, CPoint point)
{
	if(! m_bGrid 
		|| (GetStyle() & LVS_TYPEMASK) != LVS_REPORT 
		|| ! (GetExtendedStyle() & LVS_EX_FULLROWSELECT))
	{
		CListCtrl::OnRButtonDown(nFlags, point);
		return;
	}

	// Find out what subitem was clicked
	LVHITTESTINFO hitinfo = {0};
	hitinfo.flags = nFlags;
	hitinfo.pt = point;
	SubItemHitTest(&hitinfo);

	// If not right-clicking on an actual row, then don't update focus cell
	if(hitinfo.iItem == -1)
	{
		CListCtrl::OnRButtonDown(nFlags, point);
		return;
	}

	// Update the focused cell before calling CListCtrl::OnLButtonDown()
	// as it might cause a row-repaint
	m_FocusCell = hitinfo.iSubItem;
	CListCtrl::OnRButtonDown(nFlags, point);

	// CListCtrl::OnLButtonDown() doesn't always cause a row-repaint
	// call our own method to ensure the row is repainted
	UpdateFocusCell(hitinfo.iSubItem);
}

void CListCtrlExt::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Catch event before the parent listctrl gets it to avoid extra scrolling
	//	- OBS! This can also prevent the key-events to reach LVN_KEYDOWN handlers

	if(! m_bGrid 
		|| (GetStyle() & LVS_TYPEMASK) != LVS_REPORT 
		|| ! (GetExtendedStyle() & LVS_EX_FULLROWSELECT))
	{
		CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	switch(nChar)
	{
		case VK_RIGHT:	
			MoveFocusCell(TRUE);
			return;	// Do not allow scroll
		case VK_LEFT:
			MoveFocusCell(FALSE);
			return;	// Do not allow scroll
		case 0x41:	// CTRL+A (Select all rows)
			if(GetKeyState(VK_CONTROL) < 0)
			{
				if(! (GetStyle() & LVS_SINGLESEL))
				{
					SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
				}
			}
			break;
		case VK_F2:
			DisplayEditor(GetNextItem(-1, LVNI_SELECTED),m_FocusCell);
			break;
	}

	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}
// Keyboard search with subitems
void CListCtrlExt::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(m_FocusCell <= 0)
	{
		CListCtrl::OnChar(nChar, nRepCnt, nFlags);
		return;
	}

	if(GetKeyState(VK_CONTROL) < 0)
	{
		m_sLastSearchString = _T("");
		return;
	}

	// No input within 2 seconds, resets the search
	if(m_tLastSearchTime.GetCurrentTime() >= (m_tLastSearchTime + 2) && m_sLastSearchString.GetLength() > 0)m_sLastSearchString = _T("");

	// Changing cells, resets the search
	if(m_nLastSearchCell != m_FocusCell)m_sLastSearchString = _T("");

	// Changing rows, resets the search
	if(m_nLastSearchRow != GetNextItem(-1, LVNI_FOCUSED))m_sLastSearchString = _T("");

	m_nLastSearchCell = m_FocusCell;
	m_tLastSearchTime = m_tLastSearchTime.GetCurrentTime();

	if(m_sLastSearchString.GetLength() == 1 && (UINT)m_sLastSearchString.GetAt(0) == nChar)
	{
		// When the same first character is entered again,
		// then just repeat the search
	}
	else m_sLastSearchString += (TCHAR)nChar;

	int nRow = GetNextItem(-1, LVNI_FOCUSED);
	if(nRow < 0)nRow = 0;
	int nCol = m_FocusCell;
	if(nCol < 0)nCol = m_pHeaderCtrl->OrderToIndex(0);
	int nRowCount = GetItemCount();

	// Perform the search loop twice
	//	- First search from current position down to bottom
	//	- Then search from top to current position
	for(int j = 0;j < 2;++j)
	{
		for(int i = nRow + 1;i < nRowCount;++i)
		{
			CString cellText = GetItemText(i, nCol);
			if(cellText.GetLength() >= m_sLastSearchString.GetLength())
			{
				cellText = cellText.Left(m_sLastSearchString.GetLength());
				if(cellText.CompareNoCase(m_sLastSearchString) == 0)
				{
					// De-select all other rows
					SetItemState(-1, 0, LVIS_SELECTED);
					// Select row found
					SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
					// Focus row found
					SetItemState(i, LVIS_FOCUSED, LVIS_FOCUSED);	
					// Scroll to row found
					EnsureVisible(i, FALSE);			
					m_nLastSearchRow = i;
					return;
				}
			}
		}
		nRowCount = nRow;
		nRow = -1;
	}
}

void CListCtrlExt::OnNmRclickHeader(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	// TODO: Add your control notification handler code here

	CPoint point;
	GetCursorPos(&point);

	int nHeaderItemCount = m_pHeaderCtrl->GetItemCount();
	int nHeaderRemovableItemCount = m_pHeaderCtrl->GetRemovableItemCount();

	CMenu menu;
	if(nHeaderItemCount > nHeaderRemovableItemCount && 
		menu.CreatePopupMenu())
	{
		const int TEXT_LEN = 64;
		const TCHAR TEXT_TAIL[] = _T("...");
		TCHAR szText[TEXT_LEN + sizeof(TEXT_TAIL) - 1];
		HDITEM hdi;
		hdi.mask = HDI_TEXT;
		hdi.pszText = szText;
		hdi.cchTextMax = TEXT_LEN;
		int nCount = m_pHeaderCtrl->GetItemCount();
		for(int i = 0;i < nCount;++i)
		{
			if(! m_pHeaderCtrl->GetItem(i, &hdi))return;
			if(hdi.cchTextMax == TEXT_LEN - 1)_tcscat(szText, TEXT_TAIL);
			UINT nFlags = MF_STRING;
			if(! m_pHeaderCtrl->GetRemovable(i))nFlags |= MF_GRAYED | MF_CHECKED;
			if(m_pHeaderCtrl->GetVisible(i))nFlags |= MF_CHECKED;
			if(! menu.AppendMenu(nFlags, i + 1, szText))return;
		}
		UINT nIndex = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, this);
		if(nIndex > 0)
		{
			nIndex--;
			BOOL bVisible = m_pHeaderCtrl->GetVisible(nIndex);
			m_pHeaderCtrl->SetVisible(nIndex,! bVisible);
		}
	}

	*pResult = 0;
}

void CListCtrlExt::OnHdnDividerdblclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	// TODO: Add your control notification handler code here

	*pResult = 1;

	int nItem = m_pHeaderCtrl->FindVisibleItem(phdr->iItem);
	if(nItem >= 0)
	{
		int nCount = GetItemCount();
		if(nCount > 0)
		{
			CString sTemp;
			int nMaxWidth = 0;
			CClientDC dc(this);
			CFont* pOldFont = dc.SelectObject(GetFont());
			for(int i = 0;i < nCount;++i)
			{
				sTemp = GetItemText(i, nItem);
				int w = GetStringWidth(sTemp);
				nMaxWidth = max(nMaxWidth, w + 12);
			}
			CImageList* pImgList = GetImageList(LVSIL_SMALL);
			if(pImgList != NULL && nItem == 0)
			{
				int cx, cy;
				ImageList_GetIconSize(pImgList->m_hImageList, &cx, &cy);
				nMaxWidth += cx;
			}
			SetColumnWidth(nItem, nMaxWidth);
			dc.SelectObject(pOldFont);
		}
	}
}

void CListCtrlExt::OnHdnEnddrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	// TODO: Add your control notification handler code here

	m_pHeaderCtrl->PostMessage(WM_HDN_ENDDRAG, 0, 0L);
	*pResult = 0;
}

int CALLBACK CListCtrlExt::CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int nSort = 0;
	int nCompare = 0;
	ColumnInfo* pColInfo;
	CListCtrlExt* This = reinterpret_cast<CListCtrlExt*>(lParamSort);

	if(! This)return 0;
	if(! (This->m_nSortColumn < 0 || This->m_nSortColumn >= This->GetColumnCount()))
	{
		pColInfo = This->GetColumnInfo(This->m_nSortColumn);
		if(pColInfo && (pColInfo->m_eSort & SortBits))
		{
			nSort = pColInfo->m_eSort & Ascending ? 1 : -1 ;
			if(! This->m_fnCompare && pColInfo->m_fnCompare)This->m_fnCompare = pColInfo->m_fnCompare;
		}
	}
	if(This->m_fnCompare && This->m_fnCompare != &CListCtrlExt::CompareProc)
	{
		ItemData* pD1 = reinterpret_cast<ItemData*>(lParam1);
		ItemData* pD2 = reinterpret_cast<ItemData*>(lParam2);
		if(pD1)lParam1 = pD1->m_dwUserData;
		if(pD2)lParam2 = pD2->m_dwUserData;
		nCompare = This->m_fnCompare(lParam1, lParam2, This->m_dwSortData ? This->m_dwSortData : This->m_nSortColumn);
		if(! This->m_dwSortData && nSort)return nCompare * nSort;
	}
	if(! nSort)return 0;

	int nLeft = This->GetItemIndexFromData(lParam1);
	int nRight = This->GetItemIndexFromData(lParam2);

	if(nLeft < 0) nLeft = lParam1;
	if(nRight < 0) nRight = lParam2;
	int nCount = This->GetItemCount();
	if(nLeft < 0 || nRight < 0 || nLeft >= nCount || nRight >= nCount)return 0;
	nCompare = Compare(nLeft, nRight, lParamSort);

	return nCompare * nSort;
}

BOOL CListCtrlExt::SortItems(PFNLVCOMPARE pfnCompare, DWORD_PTR dwData)
{
	int nCount = GetItemCount();
	DWORD_PTR dwEditingItemData = 0;
	if(m_nEditingRow >= 0 && m_nEditingRow < nCount)dwEditingItemData = GetItemDataInternal(m_nEditingRow);
	CString dbg;
	dbg.Format(_T("\nBefore : %d"), m_nEditingRow);
	OutputDebugString(dbg);
	m_fnCompare = pfnCompare;
	m_dwSortData = dwData;
	BOOL bReturn = CListCtrl::SortItems(CompareProc, (DWORD_PTR)this);
	m_fnCompare = NULL;
	m_dwSortData = NULL;
	if(dwEditingItemData)m_nEditingRow = GetItemIndexFromData(dwEditingItemData);
	dbg.Format(_T("\nAfter : %d"), m_nEditingRow);
	OutputDebugString(dbg);

	return bReturn;
}

BOOL CListCtrlExt::SortOnColumn(int nColumn, BOOL bChangeOrder)
{
	ColumnInfo* pColInfo;
	if((pColInfo = GetColumnInfo(nColumn)) && (pColInfo->m_eSort & SortBits))
	{
		if(pColInfo->m_eSort & Auto)
		{
			pColInfo->m_eSort =(Sort)((pColInfo->m_eSort & (Ascending | Descending)) ? pColInfo->m_eSort : pColInfo->m_eSort | Descending);
			if(bChangeOrder)pColInfo->m_eSort = (Sort)(pColInfo->m_eSort ^ (Ascending | Descending));
		}
		HDITEM hd;
		hd.mask = HDI_FORMAT;
		m_pHeaderCtrl->GetItem(m_nSortColumn, &hd);
		hd.fmt = hd.fmt & ~(HDF_SORTDOWN | HDF_SORTUP);
		m_pHeaderCtrl->SetItem(m_nSortColumn, &hd);
		m_nSortColumn = nColumn;
		CListCtrl::SortItems(CompareProc, (DWORD_PTR)this);
		m_pHeaderCtrl->GetItem(nColumn, &hd);
		hd.fmt = hd.fmt & ~(HDF_SORTDOWN | HDF_SORTUP);
		hd.fmt |= pColInfo->m_eSort & Ascending ? HDF_SORTUP : HDF_SORTDOWN;
		m_pHeaderCtrl->SetItem(nColumn, &hd);
		//	to store of which column and direction was clicked
		m_bColumnSort = pColInfo->m_eSort & Ascending ? TRUE : FALSE;
		return TRUE;
	}

	return FALSE;
}

void CListCtrlExt::SetColumnSorting(int nColumn, Sort eSort, PFNLVCOMPARE fnCallBack)
{
	if(nColumn < 0 || nColumn >= GetColumnCount() || ! (eSort & SortBits))return;

	ColumnInfo* pInfo = (ColumnInfo*)GetColumnInfo(nColumn);
	if(! pInfo)SetColumnEditor(nColumn, (CWnd*)NULL);
	pInfo = (ColumnInfo*)GetColumnInfo(nColumn);
	if(pInfo)
	{
		pInfo->m_eSort = eSort;
		pInfo->m_eCompare = NotSet;
		pInfo->m_fnCompare = fnCallBack;
	}
}

int CListCtrlExt::Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CListCtrlExt* This = (CListCtrlExt*)lParamSort;
	if(! This || This->m_nSortColumn < 0 || This->m_nSortColumn >= This->GetColumnCount())return 0;
	int nSubItem = This->m_nSortColumn;
	ColumnInfo* pInfo = This->GetColumnInfo(nSubItem);
	if(! pInfo)return 0;

	CString sLeft = This->GetItemText(lParam1, nSubItem);
	CString sRight = This->GetItemText(lParam2, nSubItem);

	switch(pInfo->m_eCompare)
	{
	case Int:
		return CompareInt(sLeft, sRight);
	case Double:
		return CompareDouble(sLeft, sRight);
	case StringNoCase:
		return CompareStringNoCase(sLeft, sRight);
	case StringNumber:
		return CompareNumberString(sLeft, sRight);
	case StringNumberNoCase:
		return CompareNumberStringNoCase(sLeft, sRight);
	case String:
		return CompareString(sLeft, sRight);
	case Date:
		return CompareDate(sLeft, sRight);
	case NotSet:
		return 0;
	default:
		return CompareString(sLeft, sRight);
	}

	return CompareString(sLeft, sRight);
}

int CListCtrlExt::CompareInt(LPCTSTR pLeftText, LPCTSTR pRightText)
{
	return (int)(_ttol(pLeftText) - _ttol(pRightText));
}

int CListCtrlExt::CompareDouble(LPCTSTR pLeftText, LPCTSTR pRightText)
{
	return (int)(_tstof(pLeftText) - _tstof(pRightText));
}

int CListCtrlExt::CompareString(LPCTSTR pLeftText, LPCTSTR pRightText)
{
	return CString(pLeftText).Compare(pRightText);
}

int CListCtrlExt::CompareNumberString(LPCTSTR pLeftText, LPCTSTR pRightText)
{
	LONGLONG l1 = _ttol(pLeftText);
	LONGLONG l2 = _ttol(pRightText);

	if(l1 && l2 && (l1 - l2))
	{
		CString sTemp1, sTemp2;
		sTemp1.Format(_T("%ld"), l1);
		sTemp2.Format(_T("%ld"), l2);
		CString left(pLeftText);
		CString right(pRightText);
		if(sTemp1.GetLength() == left.GetLength() && sTemp2.GetLength() == right.GetLength())return (int)(l1 - l2);
	}

	return CString(pLeftText).Compare(pRightText);
}

int CListCtrlExt::CompareNumberStringNoCase(LPCTSTR pLeftText, LPCTSTR pRightText)
{
	LONGLONG l1 = _ttol(pLeftText);
	LONGLONG l2 = _ttol(pRightText);

	if(l1 && l2 && (l1 - l2))
	{
		CString sTemp1, sTemp2;
		sTemp1.Format(_T("%ld"),l1);
		sTemp2.Format(_T("%ld"),l2);
		CString left(pLeftText);
		CString right(pRightText);
		if(sTemp1.GetLength() == left.GetLength() && sTemp2.GetLength() == right.GetLength())return (int)(l1 - l2);
	}

	return CString(pLeftText).CompareNoCase(pRightText);
}

int CListCtrlExt::CompareStringNoCase(LPCTSTR pLeftText, LPCTSTR pRightText)
{
	return CString(pLeftText).CompareNoCase(pRightText);
}

int CListCtrlExt::CompareDate(LPCTSTR pLeftText, LPCTSTR pRightText)
{
	COleDateTime dL, dR;
	dL.ParseDateTime(pLeftText);
	dR.ParseDateTime(pRightText);

	return (dL == dR ? 0 : (dL < dR ? -1 : 1));
}

void CListCtrlExt::SetColumnSorting(int nColumn, Sort eSort, Comparer eComparer)
{
	if(nColumn < 0 || nColumn >= GetColumnCount() || ! (eSort & SortBits))return;

	ColumnInfo* pInfo = (ColumnInfo*)GetColumnInfo(nColumn);
	if(! pInfo)SetColumnEditor(nColumn, (CWnd*)NULL);
	pInfo = (ColumnInfo*)GetColumnInfo(nColumn);
	if(pInfo)
	{
		pInfo->m_eSort = eSort;
		pInfo->m_eCompare = eComparer;
		pInfo->m_fnCompare = NULL;
	}
}

void CListCtrlExt::SetColumnReadOnly(int nColumn, BOOL bReadOnly)
{
	if(nColumn < 0 || nColumn >= GetColumnCount())return;

	ColumnInfo* pInfo = (ColumnInfo*)GetColumnInfo(nColumn);
	if(! pInfo)SetColumnEditor(nColumn, (CWnd*)NULL);
	pInfo = (ColumnInfo*)GetColumnInfo(nColumn);
	if(pInfo)pInfo->m_eiEditor.m_bReadOnly = bReadOnly;
}

void CListCtrlExt::SetCellReadOnly(int nRow, int nColumn, BOOL bReadOnly )
{
	if(nRow < 0 || nRow >= GetItemCount() || nColumn < 0 || nColumn >= GetColumnCount())return;

	CellInfo* pInfo = (CellInfo*)GetCellInfo(nRow, nColumn);
	if(! pInfo)SetCellEditor(nRow, nColumn, (CWnd*)NULL);
	pInfo = (CellInfo*)GetCellInfo(nRow, nColumn);
	if(pInfo)pInfo->m_eiEditor.m_bReadOnly = bReadOnly;
}

void CListCtrlExt::SetRowReadOnly(int nRow, BOOL bReadOnly)
{
	if(nRow < 0 || nRow >= GetItemCount())return;

	ItemData* pInfo = (ItemData*)GetItemDataInternal(nRow);
	if(! pInfo)SetItemData(nRow, 0);
	pInfo = (ItemData*)GetItemDataInternal(nRow);
	if(pInfo)pInfo->m_eiEditor.m_bReadOnly = bReadOnly;
}

BOOL CListCtrlExt::IsColumnReadOnly(int nColumn)
{
	if(nColumn < 0 || nColumn >= GetColumnCount())return FALSE;

	ColumnInfo *pInfo = (ColumnInfo*)GetColumnInfo(nColumn);
	if(pInfo)return pInfo->m_eiEditor.m_bReadOnly;

	return FALSE;
}

BOOL CListCtrlExt::IsColumnHidden(int nColumn)
{
	return ! m_pHeaderCtrl->GetVisible(nColumn);
}

BOOL CListCtrlExt::IsRowReadOnly(int nRow)
{
	if(nRow < 0 || nRow >= GetItemCount())return FALSE;

	ItemData* pInfo = (ItemData*)GetItemDataInternal(nRow);
	if(pInfo)return pInfo->m_eiEditor.m_bReadOnly;

	return FALSE;
}

BOOL CListCtrlExt::IsCellReadOnly(int nRow, int nColumn)
{
	if(nRow < 0 || nRow >= GetItemCount() || nColumn < 0 || nColumn >= GetColumnCount())return FALSE;

	CellInfo* pInfo = (CellInfo*)GetCellInfo(nRow, nColumn);
	if(pInfo)return pInfo->m_eiEditor.m_bReadOnly;
	else return (IsRowReadOnly(nRow) || IsColumnReadOnly(nColumn));
}

void CListCtrlExt::SetRowColors(int nItem, COLORREF clrBk, COLORREF clrText)
{
	ItemData* pData = (ItemData*)GetItemDataInternal(nItem);
	if(! pData)SetItemData(nItem, 0);
	pData = (ItemData*)GetItemDataInternal(nItem);
	if(! pData)return;

	pData->m_clrText = clrText;
	pData->m_clrBack = clrBk;
	Update(nItem);
}

void CListCtrlExt::SetColumnColors(int nColumn, COLORREF clrBack, COLORREF clrText)
{
	if(nColumn < 0 || nColumn >= GetColumnCount())return;

	ColumnInfo* pColInfo = GetColumnInfo(nColumn);
	if(! pColInfo)
	{ 
		pColInfo = new ColumnInfo(nColumn);  
		m_aColumnInfo.Add(pColInfo);
	}

	if(pColInfo)
	{
		pColInfo->m_clrBack = clrBack;
		pColInfo->m_clrText = clrText;
	}
}

void CListCtrlExt::SetCellColors(int nRow, int nColumn, COLORREF clrBack, COLORREF clrText)
{
	if(nRow < 0 || nRow >= GetItemCount() || nColumn < 0 || nColumn >= GetColumnCount())return;

	CellInfo* pCellInfo = GetCellInfo(nRow, nColumn);
	if(! pCellInfo)SetCellData(nRow, nColumn, 0);

	pCellInfo = GetCellInfo(nRow, nColumn);
	if(pCellInfo)
	{
		pCellInfo->m_clrBack = clrBack;
		pCellInfo->m_clrText = clrText;
	}
}

void CListCtrlExt::SetColumnEditor(int nColumn, PFNEDITORCALLBACK pfnInitEditor, PFNEDITORCALLBACK pfnEndEditor, CWnd* pWnd)
{
	ColumnInfo* pColInfo = GetColumnInfo(nColumn);
	if(! pColInfo)
	{ 
		pColInfo = new ColumnInfo(nColumn);
		m_aColumnInfo.Add(pColInfo);
	}
	if(pColInfo)
	{
		pColInfo->m_eiEditor.m_pfnInitEditor = pfnInitEditor;
		pColInfo->m_eiEditor.m_pfnEndEditor = pfnEndEditor;
		pColInfo->m_eiEditor.m_pWnd = pWnd;
	}
}

void CListCtrlExt::SetColumnEditor(int nColumn, CWnd* pWnd)
{
	SetColumnEditor(nColumn, NULL, NULL, pWnd);
}

void CListCtrlExt::SetRowEditor(int nRow, CWnd* pWnd)
{
	SetRowEditor(nRow, NULL, NULL, pWnd);
}

void CListCtrlExt::SetRowEditor(int nRow, PFNEDITORCALLBACK pfnInitEditor, PFNEDITORCALLBACK pfnEndEditor, CWnd* pWnd)
{
	ItemData* pData = (ItemData*)GetItemDataInternal(nRow);
	if(! pData)
	{ 
		SetItemData(nRow, 0);
		pData = (ItemData*)GetItemDataInternal(nRow);
	}
	if(pData)
	{
		pData->m_eiEditor.m_pfnInitEditor = pfnInitEditor;
		pData->m_eiEditor.m_pfnEndEditor = pfnEndEditor;
		pData->m_eiEditor.m_pWnd = pWnd;
	}
}

BOOL CListCtrlExt::DisplayEditor(int nItem, int nSubItem)
{
	int nCount = GetItemCount();
	DWORD_PTR dwEditingItemData = 0;
	if(nItem >= 0 && nItem < nCount)dwEditingItemData = GetItemDataInternal(nItem);
	HideEditor();
	if(dwEditingItemData)nItem = GetItemIndexFromData(dwEditingItemData);
	if(nItem < 0 || nItem > nCount || nSubItem < 0 || nSubItem > GetColumnCount()
		|| IsColumnReadOnly(nSubItem) || IsRowReadOnly(nItem) || IsCellReadOnly(nItem, nSubItem) || IsColumnHidden(nSubItem))return FALSE;
	CRect rectSubItem;
//	GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rectSubItem);
	EnsureSubItemVisible(nItem, nSubItem, &rectSubItem);

	CellInfo* pCellInfo = GetCellInfo(nItem, nSubItem);
	ColumnInfo* pColInfo = GetColumnInfo(nSubItem);
	ItemData* pRowInfo = (ItemData*)GetItemDataInternal(nItem);

	BOOL bReadOnly = FALSE;
	m_pEditor = &m_eiDefEditor;
	if(pColInfo && ! (bReadOnly |= pColInfo->m_eiEditor.m_bReadOnly) && pColInfo->m_eiEditor.IsSet())m_pEditor = &pColInfo->m_eiEditor;
	if(pRowInfo && ! (bReadOnly |= pRowInfo->m_eiEditor.m_bReadOnly) && pRowInfo->m_eiEditor.IsSet())m_pEditor = &pRowInfo->m_eiEditor;
	if(pCellInfo && ! (bReadOnly |= pCellInfo->m_eiEditor.m_bReadOnly) && pCellInfo->m_eiEditor.IsSet())m_pEditor = &pCellInfo->m_eiEditor;
	if(bReadOnly || ! m_pEditor || ! m_pEditor->IsSet() || m_pEditor->m_bReadOnly)
	{
		m_pEditor = NULL;
		return FALSE;
	}

	m_nEditingRow = nItem;
	m_nEditingColumn = nSubItem;
	m_nRow = nItem;
	m_nColumn = nSubItem;
	CString sText =  GetItemText(nItem, nSubItem);
	if(m_pEditor->m_pfnInitEditor)m_pEditor->m_pfnInitEditor(&m_pEditor->m_pWnd, nItem, nSubItem, sText, GetItemData(nItem), this, TRUE);

	if(! m_pEditor->m_pWnd)return FALSE;
	SelectItem(-1, FALSE);
	if(! m_pEditor->m_pfnInitEditor)m_pEditor->m_pWnd->SetWindowText(sText);

	m_pEditor->m_pWnd->SetParent(this);
	m_pEditor->m_pWnd->SetOwner(this);

	m_pEditor->m_pWnd->SetWindowPos(NULL, rectSubItem.left, rectSubItem.top, rectSubItem.Width(), rectSubItem.Height(), SWP_SHOWWINDOW);
	m_pEditor->m_pWnd->ShowWindow(SW_SHOW);
	m_pEditor->m_pWnd->SetFocus();

	m_MsgHook.Attach(m_pEditor->m_pWnd->m_hWnd, this->m_hWnd);

	return TRUE;
}

void CListCtrlExt::HideEditor(BOOL bUpdate)
{
	CSingleLock lock(&m_oLock, TRUE);

	if(lock.IsLocked() && m_MsgHook.Detach())
	{	
		if(m_pEditor && m_pEditor->m_pWnd)
		{
			m_pEditor->m_pWnd->ShowWindow(SW_HIDE);
			CString sText;
			DWORD_PTR dwData = 0;
			if(GetItemCount() > m_nEditingRow)
			{
				sText =  GetItemText(m_nEditingRow, m_nEditingColumn);
				dwData = GetItemData(m_nEditingRow);
			}
			else bUpdate = FALSE;
			if(m_pEditor->m_pfnEndEditor)bUpdate = m_pEditor->m_pfnEndEditor(&m_pEditor->m_pWnd, m_nEditingRow, m_nEditingColumn, sText, dwData, this, bUpdate);
			else m_pEditor->m_pWnd->GetWindowText(sText);
			if(bUpdate)SetItemText(m_nEditingRow, m_nEditingColumn, sText);
			if(GetItemCount() > m_nEditingRow)Update(m_nEditingRow);
			if(bUpdate == -1)SortOnColumn(m_nEditingColumn);
			m_pEditor = NULL;
		}
	}

	lock.Unlock();
}

void CListCtrlExt::SetCellEditor(int nRow, int nColumn, PFNEDITORCALLBACK pfnInitEditor, PFNEDITORCALLBACK pfnEndEditor, CWnd* pWnd)
{
	if(nRow < 0 || nRow >= GetItemCount() || nColumn < 0 || nColumn >= GetColumnCount())return;

	CellInfo* pCellInfo = GetCellInfo(nRow, nColumn);
	if(! pCellInfo)
	{ 
		SetCellData(nRow, nColumn, 0);
		pCellInfo = (CellInfo*)GetCellInfo(nRow, nColumn);
	}

	if(pCellInfo)
	{
		pCellInfo->m_eiEditor.m_pfnInitEditor = pfnInitEditor;
		pCellInfo->m_eiEditor.m_pfnEndEditor = pfnEndEditor;
		pCellInfo->m_eiEditor.m_pWnd = pWnd;
	}
}

void CListCtrlExt::SetCellEditor(int nRow, int nColumn, CWnd* pWnd)
{
	SetCellEditor(nRow, nColumn, NULL, NULL, pWnd);
}

void CListCtrlExt::SetDefaultEditor(PFNEDITORCALLBACK pfnInitEditor, PFNEDITORCALLBACK pfnEndEditor, CWnd* pWnd)
{
	m_eiDefEditor.m_pfnInitEditor = pfnInitEditor;
	m_eiDefEditor.m_pfnEndEditor = pfnEndEditor;
	m_eiDefEditor.m_pWnd = pWnd;
}

void CListCtrlExt::SetDefaultEditor(CWnd* pWnd)
{
	SetDefaultEditor(NULL, NULL, pWnd);
}

BOOL CListCtrlExt::SaveState()
{
	CString sListName;
	CWinApp* pApp = (CWinApp*)AfxGetApp();
	sListName.Format(_T("%d"),GetDlgCtrlID());

	int nCount = m_pHeaderCtrl->GetItemCount();
	pApp->WriteProfileInt(sListName,_T("ColumnCount"),nCount);

	int* nColOrder = new int[nCount];
	m_pHeaderCtrl->GetOrderArray(nColOrder,nCount);
	pApp->WriteProfileBinary(sListName,_T("ColumnOrder"),(BYTE*)nColOrder,sizeof(int) * nCount);
	delete []nColOrder;

	int* nColWidth = new int[nCount];
	m_pHeaderCtrl->GetWidthArray(nColWidth,nCount);
	pApp->WriteProfileBinary(sListName,_T("ColumnWidth"),(BYTE*)nColWidth,sizeof(int) * nCount);
	delete []nColWidth;

	int* nColVisible = new int[nCount];
	m_pHeaderCtrl->GetVisibleArray(nColVisible,nCount);
	pApp->WriteProfileBinary(sListName,_T("ColumnVisible"),(BYTE*)nColVisible,sizeof(int) * nCount);
	delete []nColVisible;

	pApp->WriteProfileInt(sListName,_T("ColSortNum"),m_nSortColumn);
	pApp->WriteProfileInt(sListName,_T("ColSortDir"),m_bColumnSort);

	return TRUE;
}

BOOL CListCtrlExt::RestoreState()
{
	CString sListName;
	CWinApp* pApp = (CWinApp*)AfxGetApp();
	sListName.Format(_T("%d"),GetDlgCtrlID());

	int nCount = pApp->GetProfileInt(sListName,_T("ColumnCount"), 0);
	if(nCount != m_pHeaderCtrl->GetItemCount())return FALSE;

	UINT nBytes;
	BOOL bReturn = FALSE;
	int* nColOrder = NULL;
	if((pApp->GetProfileBinary(sListName,_T("ColumnOrder"),(BYTE**)&nColOrder,&nBytes) && nBytes == sizeof(int) * nCount))
	{
		bReturn = VerifyOrderArray(nColOrder,nCount) && m_pHeaderCtrl->SetOrderArray(nCount,nColOrder);
		ASSERT(bReturn);
	}
	delete []nColOrder;
	if(! bReturn)return FALSE;

	bReturn = FALSE;
	int* nColWidth = NULL;
	if((pApp->GetProfileBinary(sListName,_T("ColumnWidth"),(BYTE**)&nColWidth,&nBytes) && nBytes == sizeof(int) * nCount))
		bReturn = m_pHeaderCtrl->SetWidthArray(nCount,nColWidth);
	delete []nColWidth;
	if(! bReturn)return FALSE;

	bReturn = FALSE;
	int* nColVisible = NULL;
	if((pApp->GetProfileBinary(sListName,_T("ColumnVisible"),(BYTE**)&nColVisible,&nBytes) && nBytes == sizeof(int) * nCount))
		bReturn = m_pHeaderCtrl->SetVisibleArray(nCount,nColVisible);
	delete []nColVisible;

	int nColumnSort = pApp->GetProfileInt(sListName,_T("ColSortNum"),0);
	BOOL bColumnSort = pApp->GetProfileInt(sListName,_T("ColSortDir"),0);
	SortOnColumn(nColumnSort,bColumnSort);

	return bReturn;
}

BOOL CListCtrlExt::VerifyOrderArray(int* piArray, int nCount)
{
	for(int i = 0;i < nCount;++i)
	{
		if(! (piArray[i] >= 0 && piArray[i] <= nCount - 1))return FALSE;
		// compare with items after current one
		for(int j = i + 1;j < nCount;++j)
		{
			if(piArray[i] == piArray[j])return FALSE;
		}
	}

	return TRUE;
}

void CListCtrlExt::UpdateFocusCell(int nCol)
{
	m_FocusCell = nCol;	// Update focus cell before starting re-draw
	int nFocusRow = GetNextItem(-1, LVNI_FOCUSED);

	if(nFocusRow >= 0)
	{
		CRect itemRect;
		VERIFY(GetItemRect(nFocusRow, itemRect, LVIR_BOUNDS));
		InvalidateRect(itemRect);
		UpdateWindow();
	}
}

void CListCtrlExt::MoveFocusCell(BOOL bRight)
{
	if(GetItemCount() <= 0)
	{
		m_FocusCell = -1;	// Entire row selected
		return;
	}

	if(m_FocusCell == -1)
	{
		// Entire row already selected
		if(bRight)
		{
			// Change to the first column in the current order
			m_FocusCell = m_pHeaderCtrl->OrderToIndex(0);
		}
	}
	else
	{
		// Convert focus-cell to order index
		int nOrderIndex = -1;
		for(int i = 0;i < m_pHeaderCtrl->GetItemCount();++i)
		{
			int nCol = m_pHeaderCtrl->OrderToIndex(i);
			if(nCol == m_FocusCell)
			{
				nOrderIndex = i;
				break;
			}
		}
		// Move to the following column
		if(bRight)nOrderIndex++;
		else if(nOrderIndex > 0)nOrderIndex--;
		// Convert order-index to focus cell
		if(nOrderIndex >= 0 && nOrderIndex < m_pHeaderCtrl->GetItemCount())
		{
			int nCol = m_pHeaderCtrl->OrderToIndex(nOrderIndex);
			if(! IsColumnHidden(nCol))m_FocusCell = nCol;
		}
	}
	// Ensure the column is visible
	if(m_FocusCell >= 0)
	{
		VERIFY(EnsureColumnVisible(m_FocusCell, false));
	}

	UpdateFocusCell(m_FocusCell);
}

BOOL CListCtrlExt::EnsureColumnVisible(int nCol, bool bPartialOK)
{
	if(nCol < 0 || nCol >= m_pHeaderCtrl->GetItemCount())return FALSE;

	CRect rcHeader;
	int nScrollX = 0,nOffset = GetScrollPos(SB_HORZ);
	if(m_pHeaderCtrl->GetItemRect(nCol, rcHeader) == FALSE)return FALSE;

	CRect rcClient;
	GetClientRect(&rcClient);

	if(bPartialOK)
	{
		if((rcHeader.left - nOffset < rcClient.right) && (rcHeader.right - nOffset > 0))
		{
			return TRUE;
		}
	}
	if((rcHeader.Width() > rcClient.Width()) || (rcHeader.left - nOffset < 0))
	{
		nScrollX = rcHeader.left - nOffset;
	}
	else
	{
		if(rcHeader.right - nOffset > rcClient.right)
		{
			nScrollX = rcHeader.right - nOffset - rcClient.right;
		}
	}

	if(nScrollX != 0)
	{
		CSize size(nScrollX, 0);
		if(Scroll(size) == FALSE)return FALSE;
	}

	return TRUE;
}

BOOL CListCtrlExt::GetCellRect(int nRow, int nCol, CRect& rect)
{
	// Find the top and bottom of the cell-rectangle
	CRect rowRect,colRect;
	if(GetItemRect(nRow, rowRect, LVIR_BOUNDS) == FALSE || 
		m_pHeaderCtrl->GetItemRect(nCol, colRect) == FALSE)return FALSE;

	// Adjust for scrolling
	colRect.left -= GetScrollPos(SB_HORZ);
	colRect.right -= GetScrollPos(SB_HORZ);

	rect.left = colRect.left;
	rect.top = rowRect.top;
	rect.right = colRect.right;
	rect.bottom = rowRect.bottom;

	return TRUE;
}
