// HeaderCtrlExt.cpp : implementation of the CHeaderCtrlExt class
//

#include "stdafx.h"
#include "HeaderCtrlExt.h"
#include <atlbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT WM_HDN_ENDDRAG = ::RegisterWindowMessage(_T("{8548F4AE-4A04-4c1e-9D0C-51CAEBEEA587}"));
/////////////////////////////////////////////////////////////////////////////
// CHeaderCtrlExt
IMPLEMENT_DYNAMIC(CHeaderCtrlExt, CHeaderCtrl)

CHeaderCtrlExt::CHeaderCtrlExt()
	:m_nItemWidth(0)
	,m_nDraggingItem(-1)
	,m_bDragFullWindow(FALSE)
{
}

CHeaderCtrlExt::~CHeaderCtrlExt()
{
}

BEGIN_MESSAGE_MAP(CHeaderCtrlExt, CHeaderCtrl)
	//{{AFX_MSG_MAP(CListCtrlExt)
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CAPTURECHANGED()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(WM_HDN_ENDDRAG, OnHdnEndDrag)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHeaderCtrlExt message handlers

DWORD_PTR CHeaderCtrlExt::GetItemData(int nIndex)
{
	HDITEM hdi;
	hdi.mask = HDI_LPARAM;
	if(GetItem(nIndex, &hdi))return (DWORD_PTR)hdi.lParam;

	return NULL;
}

BOOL CHeaderCtrlExt::SetItemData(int nIndex, DWORD_PTR dwData)
{
	HDITEM hdi;
	hdi.mask = HDI_LPARAM;
	hdi.lParam = (LPARAM)dwData;

	return SetItem(nIndex, &hdi);
}

int CHeaderCtrlExt::IndexToOrder(int nIndex)
{
	int nOrder = -1;
	const int nCount = GetItemCount();
	if(nCount > 0)
	{
        int* piColOrder = new int[nCount];
        ASSERT(piColOrder);

		GetOrderArray(piColOrder, nCount);
		for(int i = 0;i < nCount;++i)
		{
			if(piColOrder[i] == nIndex)
			{
				nOrder = i;
				break;
			}
		}
        delete []piColOrder;
	}

	return nOrder;
}

void CHeaderCtrlExt::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	HDHITTESTINFO ht;
	ht.pt = point;
	int nIndex = (int)SendMessage(HDM_HITTEST, 0, (LPARAM)&ht);
	if(nIndex >= 0)
	{
		if(ht.flags == HHT_ONDIVIDER)
		{
			ReleaseCapture();
			m_nDraggingItem = nIndex;
		}
		else if (ht.flags == HHT_ONDIVOPEN)
		{
			ReleaseCapture();
			m_nDraggingItem = FindVisibleItem(nIndex);
		}

		if(m_nDraggingItem >= 0)
		{
			m_ptDragStart = point;
			BeginDragDivider();
			return;
		}
	}

	CHeaderCtrl::OnLButtonDown(nFlags, point);
}

void CHeaderCtrlExt::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if(m_nDraggingItem >= 0)
	{
		int dx = point.x - m_ptDragStart.x;
		int cxy = max(0, m_nItemWidth + dx);
		
		if(m_bDragFullWindow)SetItemWidth(m_nDraggingItem, cxy);
		else
		{
			DrawDragDivider();
			m_ptDragMove = point;
			DrawDragDivider();
		}
		return;
	}

	CHeaderCtrl::OnMouseMove(nFlags, point);
}

void CHeaderCtrlExt::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if(m_nDraggingItem >= 0)
	{
		m_ptDragEnd = point;
		EndDragDivider(TRUE);
		return;
	}

	CHeaderCtrl::OnLButtonUp(nFlags, point);
}

BOOL CHeaderCtrlExt::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

//	return CHeaderCtrl::OnSetCursor(pWnd, nHitTest, message);

	HCURSOR hCursor = NULL;

	HDHITTESTINFO ht;
	::GetCursorPos(&ht.pt);
	ScreenToClient(&ht.pt);
	int nIndex = (int)SendMessage(HDM_HITTEST, 0, (LPARAM)&ht);
	if(nIndex >= 0)
	{
		if(ht.flags == HHT_ONDIVIDER)
		{
//			hCursor = AfxGetApp()->LoadCursor(IDC_HEADER_SIZE);
            hCursor = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
			ASSERT(hCursor != NULL);
		}
		else if(ht.flags == HHT_ONDIVOPEN)
		{
			int nItem = FindVisibleItem(nIndex);
			if(nItem >= 0)
			{
				if(GetItemWidth(nItem) > 0)
				{
//					hCursor = AfxGetApp()->LoadCursor(IDC_HEADER_SIZE);
                    hCursor = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
				}
				else
				{
//					hCursor = AfxGetApp()->LoadCursor(IDC_HEADER_OPEN);
                    hCursor = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
				}
			}
		}
	}

	if(hCursor == NULL)hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);

	::SetCursor(hCursor);

	return TRUE;
}

void CHeaderCtrlExt::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	HDHITTESTINFO ht;
	::GetCursorPos(&ht.pt);
	ScreenToClient(&ht.pt);
	int nIndex = (int)SendMessage(HDM_HITTEST, 0, (LPARAM)&ht);
	if(nIndex >= 0)
	{
		if(ht.flags == HHT_ONDIVOPEN)
		{
			int nItem = FindVisibleItem(nIndex);
			if(nItem >= 0)
			{
				if(GetItemWidth(nItem) > 0)
				{
					NMHEADER nmh;
					nmh.hdr.code = HDN_DIVIDERDBLCLICK;
					nmh.hdr.hwndFrom = m_hWnd;
					nmh.hdr.idFrom = GetDlgCtrlID();
					nmh.iButton = 0;
					nmh.iItem = nItem;
					nmh.pitem = NULL;	// not used for HDN_DIVIDERDBLCLICK
					GetParent()->SendMessage(WM_NOTIFY, (WPARAM)nmh.hdr.idFrom, (LPARAM)&nmh);
				}
			}
		}
	}

	CHeaderCtrl::OnLButtonDblClk(nFlags, point);
}

LRESULT CHeaderCtrlExt::OnHdnEndDrag(WPARAM wParam, LPARAM lParam)
{
	int nCount = GetItemCount();
	int* piArray = new int[nCount];
	GetOrderArray(piArray, nCount);
	SetOrderArray(nCount, piArray);
	delete []piArray;

	return 1;
}

void CHeaderCtrlExt::OnDestroy()
{
	CHeaderCtrl::OnDestroy();

	// TODO: Add your message handler code here

	for(int i = 0;i < GetItemCount();++i)
	{
		delete (CItemData*)GetItemData(i);
		SetItemData(i, NULL);
	}
}

void CHeaderCtrlExt::OnCaptureChanged(CWnd* pWnd)
{
	// TODO: Add your message handler code here

	if(pWnd != this && m_nDraggingItem >= 0)EndDragDivider(FALSE);

	CHeaderCtrl::OnCaptureChanged(pWnd);
}

BOOL CHeaderCtrlExt::SetItemWidth(int nIndex, int nWidth)
{
	HDITEM hdi;
	memset(&hdi, 0, sizeof(hdi));
	hdi.mask = HDI_WIDTH;
	hdi.cxy = nWidth;

	return SetItem(nIndex, &hdi);
}

int CHeaderCtrlExt::GetItemWidth(int nIndex)
{
	HDITEM hdi;
	memset(&hdi, 0, sizeof(hdi));
	hdi.mask = HDI_WIDTH;
	if(GetItem(nIndex, &hdi))return hdi.cxy;

	return 0;
}

BOOL CHeaderCtrlExt::GetWidthArray(int* piArray, int nCount)
{
	HDITEM hdi;
	memset(&hdi, 0, sizeof(hdi));
	hdi.mask = HDI_WIDTH;
	for(int i = 0;i < nCount;++i)
	{
		if(! GetItem(i, &hdi))return FALSE;
		piArray[i] = hdi.cxy;
	}

	return TRUE;
}

BOOL CHeaderCtrlExt::SetWidthArray(int nCount, int* piArray)
{
	HDITEM hdi;
	memset(&hdi, 0, sizeof(hdi));
	hdi.mask = HDI_WIDTH;
	for(int i = 0;i < nCount;++i)
	{
		hdi.cxy = piArray[i];
		if(! SetItem(i, &hdi))return FALSE;
	}

	return TRUE;
}

BOOL CHeaderCtrlExt::GetVisibleArray(int* piArray, int nCount)
{
	for(int i = 0;i < nCount;++i)piArray[i] = GetVisible(i);

	return TRUE;
}

BOOL CHeaderCtrlExt::SetVisibleArray(int nCount, int* piArray)
{
	for(int i = 0;i < nCount;++i)SetVisible(i, piArray[i]);

	return TRUE;
}

BOOL CHeaderCtrlExt::SetOrderArray(int nCount, LPINT piArray)
{
	// move all hidden items to the end
	int* piArrayNew = new int[nCount];
	int* p1 = piArrayNew;
	int* p2 = piArrayNew + (nCount - 1);
	for(int i = 0;i < nCount;++i)
	{
		int nIndex = piArray[i];
		if(GetVisible(nIndex))*p1++ = nIndex;
		else *p2-- = nIndex;
	}

	BOOL bReturn = CHeaderCtrl::SetOrderArray(nCount, piArrayNew);
	delete []piArrayNew;

	GetParent()->Invalidate();

	return bReturn;
}

BOOL CHeaderCtrlExt::GetVisible(int nIndex)
{
	CItemData* pData = (CItemData*)GetItemData(nIndex);
	if(pData)return pData->m_bVisible;
	else return FALSE;
}

void CHeaderCtrlExt::SetVisible(int nIndex, BOOL bVisible)
{
	CItemData* pData = (CItemData*)GetItemData(nIndex);

	if(! pData)return;

	if(pData->m_bVisible != bVisible)
	{
		pData->m_bVisible = bVisible;
		// get total items
		int nCount = GetItemCount();
		// get current item's order
		int nOrder = IndexToOrder(nIndex);
		int* piCols = new int[nCount];
		ASSERT(piCols);

		GetOrderArray(piCols, nCount);
		if(bVisible)
		{
			// restore item width
			ResetItemWidth(nIndex);
			// move the item to the original position
			int nTarget;
			int nVisible = GetVisibleItemCount();
			if(nIndex > nVisible - 1)nTarget = nVisible - 1;
			else nTarget = nIndex;

			ASSERT(nTarget <= nOrder);

			for(int i = nOrder;i > nTarget;--i)piCols[i] = piCols[i - 1];
			piCols[nTarget] = nIndex;
		}
		else
		{
			// hide item
			SetItemWidth(nIndex, 0);
		}

		SetOrderArray(nCount, piCols);
		delete []piCols;
	}
}

int CHeaderCtrlExt::FindVisibleItem(int nIndex)
{
	if(GetVisible(nIndex))return nIndex;

	int nOrder = IndexToOrder(nIndex);
	while(nOrder > 0)
	{
		nIndex = OrderToIndex(--nOrder);
		if(GetVisible(nIndex))return nIndex;
	}

	return -1;
}

int CHeaderCtrlExt::GetVisibleItemCount()
{
	int nVisible = 0;
	int nCount = GetItemCount();
	for(int i = 0;i < nCount;++i)
		if(GetVisible(i))nVisible++;

	return nVisible;
}

int CHeaderCtrlExt::GetRemovableItemCount()
{
	int nRemovable = 0;
	int nCount = GetItemCount();
	for(int i = 0;i < nCount;++i)
	{
		if(GetRemovable(i))nRemovable++;
	}

	return nRemovable;
}

BOOL CHeaderCtrlExt::GetRemovable(int nIndex)
{
	CItemData* pData = (CItemData*)GetItemData(nIndex);
	if(pData)return pData->m_bRemovable;
	else return FALSE;
}

void CHeaderCtrlExt::SetRemovable(int nIndex, BOOL bRemovable)
{
	CItemData* pData = (CItemData*)GetItemData(nIndex);
	if(pData)pData->m_bRemovable = bRemovable;
}

void CHeaderCtrlExt::BeginDragDivider()
{
	SetCapture();

	m_nItemWidth = GetItemWidth(m_nDraggingItem);

	// check if dragging full window is enabled
	::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &m_bDragFullWindow, 0);
	m_ptDragMove = m_ptDragStart;
	if(! m_bDragFullWindow)DrawDragDivider();
}

void CHeaderCtrlExt::EndDragDivider(BOOL bSubmit)
{
	if(! m_bDragFullWindow)DrawDragDivider();

	if(bSubmit)
	{
		if(! m_bDragFullWindow)
		{
			int dx = m_ptDragEnd.x - m_ptDragStart.x;
			int cxy = max(0, m_nItemWidth + dx);
			SetItemWidth(m_nDraggingItem, cxy);
		}
	}
	else
	{
		// cancel and restore the original width
		if(m_bDragFullWindow)SetItemWidth(m_nDraggingItem, m_nItemWidth);
	}

	m_nDraggingItem = -1;
	ReleaseCapture();
}

void CHeaderCtrlExt::DrawDragDivider()
{
	CWnd* pParent = GetParent();

	CDC* pDC = pParent->GetDCEx(NULL, DCX_WINDOW | DCX_CACHE | DCX_LOCKWINDOWUPDATE);
	int nROP2 = pDC->SetROP2(R2_NOT);

	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* pPenOld = pDC->SelectObject(&pen);

	CRect rc;
	pParent->GetWindowRect(&rc);
	ScreenToClient(&rc);

	CRect rcItem;
	GetItemRect(m_nDraggingItem, &rcItem);

	int dx = m_ptDragMove.x - m_ptDragStart.x;
	int x = max(rcItem.left, rcItem.right + dx);
	pDC->MoveTo(x, rc.top);
	pDC->LineTo(x, rc.bottom);

	pDC->SelectObject(pPenOld);
	pDC->SetROP2(nROP2);
	pParent->ReleaseDC(pDC);
}

int CHeaderCtrlExt::ItemFromPoint(CPoint point)
{
	for(int i = 0;i < GetItemCount();++i)
	{
		CRect rc;
		GetItemRect(i, &rc);
		if(rc.PtInRect(point))return i;
	}

	return -1;
}

void CHeaderCtrlExt::ResetItemWidth(int nIndex)
{
	CItemData* pData = (CItemData*)GetItemData(nIndex);
	if(pData)SetItemWidth(nIndex, pData->m_nWidth);
}
