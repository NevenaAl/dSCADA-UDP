// DemoAUBDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DemoAUB.h"
#include "DemoAUBDlg.h"
#include "mmsystem.h"
#include "dSim.h"

#include "Simulator/Simulator.h"

static bool InitDone = false;
bool ServerShutdown = false;
HANDLE DebugTaskHnd, VremenskiTaskHnd, KomunikacioniTaskHnd, AkvizicioniTaskHnd, RefreshTaskHnd;
HANDLE PipeTaskHnd, TandemTaskHnd;

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

    // Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDemoAUBDlg dialog

CDemoAUBDlg::CDemoAUBDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CDemoAUBDlg::IDD, pParent)
{
    // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDemoAUBDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TEXT5, m_txt5);
    DDX_Control(pDX, IDC_TEXT4, m_txt4);
    DDX_Control(pDX, IDC_TEXT3, m_txt3);
    DDX_Control(pDX, IDC_TEXT2, m_txt2);
    DDX_Control(pDX, IDC_TEXT1, m_txt1);
    DDX_Control(pDX, IDC_DBG2, m_dbg2);
    DDX_Control(pDX, IDC_DBG1, m_dbg1);
    DDX_Control(pDX, IDC_LISTA, m_lista);
}

BEGIN_MESSAGE_MAP(CDemoAUBDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_QUERYDRAGICON()
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_CLREVENTS, &CDemoAUBDlg::OnBnClickedClrevents)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDemoAUBDlg message handlers

static CDemoAUBDlg *dlg;

// kontrolni ispis
HANDLE wputs_mutex;                          // Mutex za sinhronizaciju pristupa
void wputs( int i, char *format, ... )
{
    char sbuf[400];
    va_list argptr;

    if( !ServerShutdown)
    {
        WaitForSingleObject( wputs_mutex, INFINITE );

        va_start( argptr, format );
        vsprintf( sbuf, format, argptr );
        va_end( argptr );

        switch( i )
        {
        case  1:  dlg->m_txt1.SetWindowText( sbuf );  break;
        case  2:  dlg->m_txt2.SetWindowText( sbuf );  break;
        case  3:  dlg->m_txt3.SetWindowText( sbuf );  break;
        case  4:  dlg->m_txt4.SetWindowText( sbuf );  break;
        case  5:  dlg->m_txt5.SetWindowText( sbuf );  break;
        }

        ReleaseMutex( wputs_mutex );
    }
}

// ispis u listu
HANDLE lputs_mutex;                          // Mutex za sinhronizaciju pristupa
void lputs( char *format, ... )
{   char sbuf[400];
va_list argptr;

WaitForSingleObject( lputs_mutex, INFINITE );

va_start( argptr, format );
vsprintf( sbuf, format, argptr );
va_end( argptr );

int nextTopIndex = dlg->m_lista.GetTopIndex() + 1;

if( dlg->m_lista.GetCount() > 100 )
{
    //dlg->m_lista.ResetContent();
    for( int i=0; i<20; i++ )
    {
        dlg->m_lista.DeleteString( i );
        nextTopIndex -= 1;
    }
}

dlg->m_lista.AddString( sbuf );
dlg->m_lista.SetTopIndex( nextTopIndex );

ReleaseMutex( lputs_mutex );
}

// timer funkcija
MMRESULT timerID;
void CALLBACK Timer1( UINT timerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2 )
{
    UpdateTimers();
    TimeMS += sys_timer_period;
    if( TimeMS >= 1000 )
    {
        TimeMS = 0;
        if( InitDone )
        {
            ReleaseSemaphore( periodicProcessingTimer, 1, NULL );    // otpusta vremenski task
            ReleaseSemaphore( smACQ, 1, NULL );    // otpusta simulacioni task
        }
    }
}

// funkcije za izlaz
void Ispad( void )
{
    disable_WD();
    timeKillEvent( timerID );
    WSACleanup(); 
}

void _abort( char* file, int line )
{
    char *f;
    if( f=strrchr(file, '\\') )
        f++;
    else 
        f = file;
    // zapisi i prekini rad ...
    PutLogMessage( "Abort %s|%d !", f, line );
    Ispad();
    exit( EXIT_FAILURE  );         
}


/*-----------------------------------------------------------------------*/
/*                      TASK6                                            */
/*   Task za debug ispise i testove                                      */
/*-----------------------------------------------------------------------*/
DWORD WINAPI DebugTask( LPVOID pData )
{
    char msg[100];

    // prikazi ime konfiguracije
    sprintf( msg, "[ %s : %s ]: %d variables", StnCfg.StaName, rtdb.sys_name, total.all );
    // prikazi ime konfiguracije
    dlg->m_dbg2.SetWindowText( msg );

    while( !ServerShutdown )
    {
        // osvezi WD iz najmanje prioritetnog zadatka
        refresh_WD();

        // ispisi trenutno vreme
        sprintf( msg, "CurrTime = %s", ctime2str() );
        dlg->m_dbg1.SetWindowText( msg );

        Sleep( 1000 );
    }	
    return 0;
}

void CDemoAUBDlg::StartScadaTasks()
{
    veto();        // spreci pokretanje formiranih zadataka

    if( !(VremenskiTaskHnd = CreateThread( NULL, 0, PeriodicProcessingTask, NULL, 0, NULL)) )
        _Abort();
    SetThreadPriority( VremenskiTaskHnd, THREAD_PRIORITY_HIGHEST );

    if( !(AkvizicioniTaskHnd = CreateThread( NULL, 0, SimulationTask, NULL, 0, NULL)) )
        _Abort();
    SetThreadPriority( AkvizicioniTaskHnd, THREAD_PRIORITY_NORMAL );

    if( !(PipeTaskHnd = CreateThread( NULL, 0, PipeTask, NULL, 0, NULL)) )
        _Abort();
    SetThreadPriority( PipeTaskHnd, THREAD_PRIORITY_BELOW_NORMAL );

    if( !(DebugTaskHnd = CreateThread( NULL, 0, DebugTask, NULL, 0, NULL)) )
        _Abort();
    SetThreadPriority( DebugTaskHnd, THREAD_PRIORITY_LOWEST );

    if( !(RefreshTaskHnd = CreateThread( NULL, 0, RefreshTask, NULL, 0, NULL)) )
        _Abort();
    SetThreadPriority( RefreshTaskHnd, THREAD_PRIORITY_LOWEST );

    permit( THREAD_PRIORITY_NORMAL );
}


BOOL CDemoAUBDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Add "About..." menu item to system menu.
    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // TODO: Add extra initialization here
    dlg = this;

    // Set the icon for this dialog
    m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON2));
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    CString wtext;
    wtext.Format( "dSim: %s", StnCfg.StaName );
    SetWindowText( wtext );


    LOGFONT lf;                                  // Used to create the CFont.
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight = 13;
    lf.lfWeight = FW_LIGHT;
    lf.lfQuality = CLEARTYPE_QUALITY;
    strcpy(lf.lfFaceName, "Lucida Console");     
    m_font.CreateFontIndirect( &lf );         // Create the font.
    m_lista.SetFont( &m_font );

    // inicijalizuj sistemski timer
    if( !(timerID = timeSetEvent(sys_timer_period, 1, Timer1, 0, TIME_PERIODIC)) )   // za TimeMS
        EndDialog( -1 );

    /*----------------------------------------------------------*/
    /*  Pocetne vrednosti semafora                              */
    /*----------------------------------------------------------*/
    if( (periodicProcessingTimer = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL))==NULL )
        _Abort();

    if( (smCOM = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL))==NULL )
        _Abort();

    if( (smACQ = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL))==NULL )
        _Abort();

    LogerMTX = CreateMutex( NULL, FALSE, NULL );
    wputs_mutex = CreateMutex( NULL, FALSE, NULL );
    lputs_mutex = CreateMutex( NULL, FALSE, NULL );

    /*----------------------------------------------------------*/
    /*  Pokretanje taskova                                      */
    /*----------------------------------------------------------*/

    StartScadaTasks();

    enable_WD();

    // sad je sve spremno da pocne
    InitDone = true;

    return TRUE;
}


//****************************************************

void CDemoAUBDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

void CDemoAUBDlg::OnOK() 
{
    ServerShutdown = true;
    Sleep( 1000 );
    Ispad();
    CDialog::OnOK();
}

void CDemoAUBDlg::OnClose()
{
    ServerShutdown = true;
    Sleep( 1000 );
    Ispad();
    CDialog::OnClose();
}

void CDemoAUBDlg::OnBnClickedClrevents()
{
    dlg->m_lista.ResetContent();
}
