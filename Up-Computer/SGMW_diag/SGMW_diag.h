
// SGMW_diag.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSGMW_diagApp:
// �йش����ʵ�֣������ SGMW_diag.cpp
//

class CSGMW_diagApp : public CWinApp
{
public:
	CSGMW_diagApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSGMW_diagApp theApp;