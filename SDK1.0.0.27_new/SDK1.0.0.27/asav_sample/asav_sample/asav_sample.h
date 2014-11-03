// asav_sample.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Casav_sampleApp:
// See asav_sample.cpp for the implementation of this class
//

class Casav_sampleApp : public CWinApp
{
public:
	Casav_sampleApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Casav_sampleApp theApp;