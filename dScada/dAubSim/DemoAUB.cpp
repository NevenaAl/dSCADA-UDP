// DemoAUB.cpp : Defines the class behaviors for the application.
//
#include "stdafx.h"
#include "DemoAUB.h"
#include "DemoAUBDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDemoAUBApp

BEGIN_MESSAGE_MAP(CDemoAUBApp, CWinApp)
    //{{AFX_MSG_MAP(CDemoAUBApp)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDemoAUBApp construction

CDemoAUBApp::CDemoAUBApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDemoAUBApp object

CDemoAUBApp theApp;


/////////////////////////////////////////////////////////////////////////////
// CDemoAUBApp initialization

BOOL CDemoAUBApp::InitInstance()
{
    if (!AfxSocketInit())
    {
        AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
        return FALSE;
    }

    AfxEnableControlContainer();

    if( initAUB() == OK )
    {
        CDemoAUBDlg dlg;
        m_pMainWnd = &dlg;
        dlg.DoModal();
    }

    return FALSE;
}


int CDemoAUBApp::ExitInstance()
{
    //WrASCIICfg( "SaveCfg.txt" );

    WSACleanup();
    deinitAUB();
    return OK;
}

